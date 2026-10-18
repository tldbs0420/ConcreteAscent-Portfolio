// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/ConcreteAscentCharacter.h"
#include "Player/Controller/ConcreteAscentPlayerController.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "InputAction.h"
#include "Player/Components/ParkourTraversalComponent.h"
#include "Components/CapsuleComponent.h"
#include "MotionWarpingComponent.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"

namespace
{
	const FName FrontLedgeWarpTargetName(TEXT("FrontLedge"));
	const FName BackLedgeWarpTargetName(TEXT("BackLedge"));
	const FName BackFloorWarpTargetName(TEXT("BackFloor"));
	const FName ClimbStandWarpTargetName(TEXT("ClimbStand"));

	FTransform MakeTraversalWarpTransformFromForward(const FVector& Location, const FVector& Forward)
	{
		FVector SafeForward = Forward.GetSafeNormal();

		if (SafeForward.IsNearlyZero())
			SafeForward = FVector::ForwardVector;

		FRotator Rotation = SafeForward.Rotation();
		Rotation.Pitch = 0.f;
		Rotation.Roll = 0.f;

		return FTransform(Rotation, Location, FVector::OneVector);
	}
}

AConcreteAscentCharacter::AConcreteAscentCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	MotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarpingComponent"));

	CameraArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraArm->SetupAttachment(GetRootComponent());
	CameraArm->SocketOffset = FVector(0.0f, 55.0f, 65.0f);
	CameraArm->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraArm, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
}

void AConcreteAscentCharacter::BeginPlay()
{
	Super::BeginPlay();

	ParkourTraversalComponent = FindComponentByClass<UParkourTraversalComponent>();
	if (!ParkourTraversalComponent)
		UE_LOG(LogTemp, Warning, TEXT("ParkourTraversalComponent is not attached to this character."));
}

void AConcreteAscentCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateGait();
}

void AConcreteAscentCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AConcreteAscentCharacter::Move);
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &AConcreteAscentCharacter::Move);
		}

		if (LookAction)
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AConcreteAscentCharacter::Look);

		if (JumpInputAction)
			EnhancedInputComponent->BindAction(JumpInputAction, ETriggerEvent::Started, this, &AConcreteAscentCharacter::Jump);

		if (WalkInputAction)
			EnhancedInputComponent->BindAction(WalkInputAction, ETriggerEvent::Started, this, &AConcreteAscentCharacter::ToggleWalk);

		if (SprintInputAction)
		{
			EnhancedInputComponent->BindAction(SprintInputAction, ETriggerEvent::Started, this, &AConcreteAscentCharacter::StartSprint);
			EnhancedInputComponent->BindAction(SprintInputAction, ETriggerEvent::Completed, this, &AConcreteAscentCharacter::StopSprint);
			EnhancedInputComponent->BindAction(SprintInputAction, ETriggerEvent::Canceled, this, &AConcreteAscentCharacter::StopSprint);
		}

		if (PauseInputAction)
			EnhancedInputComponent->BindAction(PauseInputAction, ETriggerEvent::Started, this, &AConcreteAscentCharacter::TogglePauseMenu);
	}
}

void AConcreteAscentCharacter::Landed(const FHitResult& Hit)
{
	const float LandingSpeed = FMath::Abs(GetVelocity().Z);

	Super::Landed(Hit);

	LastLandingVerticalSpeed = LandingSpeed;
	bJustLanded = true;

	GetWorldTimerManager().ClearTimer(JustLandedTimerHandle);
	GetWorldTimerManager().SetTimer(JustLandedTimerHandle, this, &AConcreteAscentCharacter::ClearJustLanded, 0.3f, false);
}

void AConcreteAscentCharacter::SnapToCurrentLedgeHang()
{
	if (!CurrentTraversalResult.bHasFrontLedge)
	{
		UE_LOG(LogTemp, Warning, TEXT("SnapToCurrentLedgeHang failed: No front ledge."));
		return;
	}

	SetHangLocationFromLedgePoint(CurrentTraversalResult.FrontLedgeLocation, CurrentTraversalResult.FrontLedgeNormal);

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->StopMovementImmediately();
		MoveComp->Velocity = FVector::ZeroVector;
		MoveComp->SetMovementMode(MOVE_Flying);
		MoveComp->MaxFlySpeed = 0.f;
		MoveComp->MaxWalkSpeed = 0.f;
	}
}

void AConcreteAscentCharacter::SetHangLocationFromLedgePoint(const FVector& LedgePoint, const FVector& LedgeNormal)
{
	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	if (!CapsuleComp)
		return;

	FVector Outward = LedgeNormal;
	Outward.Z = 0.f;

	if (!Outward.Normalize())
	{
		Outward = -GetActorForwardVector();
		Outward.Z = 0.f;
		Outward.Normalize();
	}

	if (Outward.IsNearlyZero())
		Outward = FVector::BackwardVector;

	const FVector FacingWall = -Outward;
	const float CapsuleRadius = CapsuleComp->GetScaledCapsuleRadius();

	const FVector HangActorLocation = LedgePoint + Outward * (CapsuleRadius + HangWallGap) - FVector(0.f, 0.f, HangRootZOffset);

	SetActorLocationAndRotation(HangActorLocation, FacingWall.Rotation(), false, nullptr, ETeleportType::TeleportPhysics);
}

void AConcreteAscentCharacter::SetClimbStartLocationFromLedgePoint(const FVector& LedgePoint, const FVector& LedgeNormal)
{
	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	if (!CapsuleComp)
		return;

	FVector Outward = LedgeNormal;
	Outward.Z = 0.f;

	if (!Outward.Normalize())
	{
		Outward = -GetActorForwardVector();
		Outward.Z = 0.f;
		Outward.Normalize();
	}

	if (Outward.IsNearlyZero())
		Outward = FVector::BackwardVector;

	const FVector FacingWall = -Outward;
	const float CapsuleRadius = CapsuleComp->GetScaledCapsuleRadius();

	const FVector ClimbStartActorLocation = 
		LedgePoint 
		+ Outward * (CapsuleRadius + ClimbStartWallGap) 
		- FVector(0.f, 0.f, ClimbStartRootZOffset);

	SetActorLocationAndRotation(ClimbStartActorLocation, FacingWall.Rotation(), false, nullptr, ETeleportType::TeleportPhysics);

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->StopMovementImmediately();
		MoveComp->Velocity = FVector::ZeroVector;
		MoveComp->SetMovementMode(MOVE_Flying);
		MoveComp->MaxFlySpeed = 0.f;
		MoveComp->MaxWalkSpeed = 0.f;
	}
}

void AConcreteAscentCharacter::SetClimbStandWarpTarget(const FVector& StandLocation, const FRotator& StandRotation)
{
	if (!MotionWarpingComponent)
		return;

	MotionWarpingComponent->AddOrUpdateWarpTargetFromTransform(ClimbStandWarpTargetName, FTransform(StandRotation, StandLocation, FVector::OneVector));
}

void AConcreteAscentCharacter::ClearClimbStandWarpTarget()
{
	if (!MotionWarpingComponent)
		return;

	MotionWarpingComponent->RemoveWarpTarget(ClimbStandWarpTargetName);
}

void AConcreteAscentCharacter::EnterLedgeClimbState()
{
	bIsHanging = true;
	bIsTraversing = true;
	bCanMove = false;
	bSprint = false;

	bWasHangingMoveInputPressed = false;
	bWasHangingVerticalInputPressed = false;

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		// Climb 중 실제 이동은 Root Motion + Motion Warping이 담당
		MoveComp->StopMovementImmediately();
		MoveComp->Velocity = FVector::ZeroVector;
		MoveComp->SetMovementMode(MOVE_Flying);
		MoveComp->MaxWalkSpeed = 0.f;
		MoveComp->MaxFlySpeed = 0.f;
	}
}

void AConcreteAscentCharacter::EnterHangingState()
{
	bIsTraversing = false;
	bIsHanging = true;
	bCanMove = false;
	bSprint = false;
	bWasHangingMoveInputPressed = false;
	bWasHangingVerticalInputPressed = false;

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->StopMovementImmediately();
		MoveComp->Velocity = FVector::ZeroVector;
		MoveComp->SetMovementMode(MOVE_Flying);

		MoveComp->MaxWalkSpeed = 0.f;
		MoveComp->MaxFlySpeed = 0.f;
	}

	BP_OnEnterHangingState();
}

void AConcreteAscentCharacter::ExitHangingToFalling()
{
	bIsHanging = false;
	bIsTraversing = false;
	bCanMove = true;
	bSprint = false;
	bWasHangingMoveInputPressed = false;
	bWasHangingVerticalInputPressed = false;

	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
			AnimInstance->Montage_Stop(0.05f, nullptr);
	}

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->StopMovementImmediately();
		MoveComp->Velocity = FVector::ZeroVector;

		MoveComp->MaxWalkSpeed = RunMaxSpeed;
		MoveComp->MaxFlySpeed = RunMaxSpeed;

		MoveComp->SetMovementMode(MOVE_Falling);
	}

	UpdateGait();
}

void AConcreteAscentCharacter::ExitHangingToStanding(const FVector& StandLocation, const FRotator& StandRotation)
{
	bIsHanging = false;
	bIsTraversing = false;
	bCanMove = true;
	bSprint = false;
	bWasHangingMoveInputPressed = false;
	bWasHangingVerticalInputPressed = false;

	// 오차 보정용
	SetActorLocationAndRotation(StandLocation, StandRotation, false, nullptr, ETeleportType::TeleportPhysics);
	ClearClimbStandWarpTarget();

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->StopMovementImmediately();
		MoveComp->Velocity = FVector::ZeroVector;

		MoveComp->MaxWalkSpeed = RunMaxSpeed;
		MoveComp->MaxFlySpeed = RunMaxSpeed;

		MoveComp->SetMovementMode(MOVE_Walking);
	}

	UpdateGait();
}

void AConcreteAscentCharacter::UpdateGait()
{
	if (bIsTraversing || bIsHanging)
		return;

	UCharacterMovementComponent* AscentCharacterMovement = GetCharacterMovement();
	if (!AscentCharacterMovement)
		return;

	if (!bCanMove)
	{
		AscentCharacterMovement->MaxWalkSpeed = 0.f;
		AscentCharacterMovement->MaxFlySpeed = 0.f;
		return;
	}

	if (bSprint)
	{
		Gait = EGait::Sprint;
		AscentCharacterMovement->MaxWalkSpeed = SprintMaxSpeed;
	}
	else if (bWalk)
	{
		Gait = EGait::Walk;
		AscentCharacterMovement->MaxWalkSpeed = WalkMaxSpeed;
	}
	else
	{
		Gait = EGait::Run;
		AscentCharacterMovement->MaxWalkSpeed = RunMaxSpeed;
	}
}

void AConcreteAscentCharacter::ClearJustLanded()
{
	bJustLanded = false;
}

void AConcreteAscentCharacter::BeginTraversal()
{
	bIsTraversing = true;
	bCanMove = false;

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->StopMovementImmediately();
		MoveComp->SetMovementMode(MOVE_Flying);
	}

	if (UCapsuleComponent* CapsuleComp = GetCapsuleComponent())
	{
		if (UPrimitiveComponent* HitComponent = CurrentTraversalResult.HitComponent.Get())
			CapsuleComp->IgnoreComponentWhenMoving(HitComponent, true);
	}
}

void AConcreteAscentCharacter::EndTraversal()
{
	bIsTraversing = false;
	bCanMove = true;

	if (UCapsuleComponent* CapsuleComp = GetCapsuleComponent())
	{
		if (CurrentTraversalResult.HitComponent)
			CapsuleComp->IgnoreComponentWhenMoving(CurrentTraversalResult.HitComponent.Get(), false);
	}

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		switch (CurrentTraversalResult.ActionType)
		{
		case ETraversalAction::Vault:
			MoveComp->SetMovementMode(MOVE_Falling);
			break;

		case ETraversalAction::Hurdle:
		case ETraversalAction::Mantle:
		default:
			MoveComp->SetMovementMode(MOVE_Walking);
			break;
		}
	}

	UpdateGait();
}

void AConcreteAscentCharacter::OnTraversalMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	// 몽타주가 끝나거나 중단되면 파쿠르 상태를 정리한다.
	EndTraversal();
}

void AConcreteAscentCharacter::Move(const FInputActionValue& InputValue)
{
	const FVector2D MovementVector = InputValue.Get<FVector2D>();

	if (bIsTraversing)
		return;

	if (bIsHanging)
	{
		const bool bHorizontalPressed = FMath::Abs(MovementVector.X) >= 0.5f;
		const bool bVerticalPressed = FMath::Abs(MovementVector.Y) >= 0.5f;

		if (!bHorizontalPressed)
			bWasHangingMoveInputPressed = false;

		if (!bVerticalPressed)
			bWasHangingVerticalInputPressed = false;

		if (bVerticalPressed)
		{
			if (bWasHangingVerticalInputPressed)
				return;

			bWasHangingVerticalInputPressed = true;

			if (ParkourTraversalComponent)
			{
				if (MovementVector.Y > 0.f)
					// W: 위로 기어오르기
					ParkourTraversalComponent->ClimbFromLedge();
				else
					// S: 놓고 떨어지기
					ParkourTraversalComponent->DropFromLedge();
			}

			return;
		}

		if (bHorizontalPressed)
		{
			if (bWasHangingMoveInputPressed)
				return;

			bWasHangingMoveInputPressed = true;

			if (ParkourTraversalComponent)
				ParkourTraversalComponent->MoveAlongLedge(MovementVector.X);

			return;
		}

		return;
	}

	bWasHangingMoveInputPressed = false;
	bWasHangingVerticalInputPressed = false;

	if (!bCanMove)
		return;

	const FRotator MovementRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);

	if (MovementVector.Y != 0.0f)
	{
		const FVector ForwardDirection = MovementRotation.RotateVector(FVector::ForwardVector);
		AddMovementInput(ForwardDirection, MovementVector.Y);
	}

	if (MovementVector.X != 0.0f)
	{
		const FVector RightDirection = MovementRotation.RotateVector(FVector::RightVector);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AConcreteAscentCharacter::Look(const FInputActionValue& InputValue)
{
	const FVector2D LookAxisVector = InputValue.Get<FVector2D>();

	if (LookAxisVector.X != 0.0f)
		AddControllerYawInput(LookAxisVector.X);
	if (LookAxisVector.Y != 0.0f)
		AddControllerPitchInput(LookAxisVector.Y);
}

void AConcreteAscentCharacter::ToggleWalk(const FInputActionValue& InputValue)
{
	bWalk = !bWalk;
}

void AConcreteAscentCharacter::StartSprint(const FInputActionValue& InputValue)
{
	bSprint = true;
}

void AConcreteAscentCharacter::StopSprint(const FInputActionValue& InputValue)
{
	bSprint = false;
}

void AConcreteAscentCharacter::TogglePauseMenu(const FInputActionValue& InputValue)
{
	AConcreteAscentPlayerController* AscentController = Cast<AConcreteAscentPlayerController>(GetController());
	if (!AscentController)
		return;

	AscentController->TogglePauseMenu();
}

void AConcreteAscentCharacter::Jump()
{
	if (bIsHanging || bIsTraversing)
		return;

	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	const bool bIsInAir = MoveComp && MoveComp->IsFalling();

	// 공중에서 Space를 한 번 더 누르면 난간 잡기를 시도한다.
	if (bIsInAir)
	{
		if (ParkourTraversalComponent)
		{
			FTraversalCheckResult LedgeGrabResult;

			if (ParkourTraversalComponent->TryAirLedgeGrab(LedgeGrabResult))
			{
				CurrentTraversalResult = LedgeGrabResult;

				SnapToCurrentLedgeHang();
				EnterHangingState();

				return;
			}
		}
	}

	// 지상에서는 파쿠르 / 일반 점프 흐름
	if (ParkourTraversalComponent)
	{
		FTraversalCheckResult TraversalResult;

		if (ParkourTraversalComponent->StartTraversal(TraversalResult))
		{
			CurrentTraversalResult = TraversalResult;
			BeginTraversal();
			UpdateTraversalWarpTargets(CurrentTraversalResult);

			const float MontageLength = PlayTraversalMontage(CurrentTraversalResult.ChosenMontage, CurrentTraversalResult.PlayRate, CurrentTraversalResult.StartTime);
			if (MontageLength > 0.f)
				return;

			EndTraversal();
		}
	}

	Super::Jump();
}

void AConcreteAscentCharacter::RespawnAt(const FTransform& RespawnTransform)
{
	SetActorLocationAndRotation(RespawnTransform.GetLocation(), RespawnTransform.GetRotation().Rotator(), false, nullptr, ETeleportType::TeleportPhysics);

	if (UCharacterMovementComponent* AscentCharacterMovement = GetCharacterMovement())
	{
		AscentCharacterMovement->StopMovementImmediately();
		AscentCharacterMovement->Velocity = FVector::ZeroVector;
		AscentCharacterMovement->SetMovementMode(MOVE_Walking);
		AscentCharacterMovement->MaxWalkSpeed = RunMaxSpeed;
		AscentCharacterMovement->MaxFlySpeed = RunMaxSpeed;
	}

	GetWorldTimerManager().ClearTimer(JustLandedTimerHandle);

	bCanMove = true;
	bIsTraversing = false;
	bIsHanging = false;
	bJustLanded = false;
	bSprint = false;
	bWasHangingMoveInputPressed = false;
	bWasHangingVerticalInputPressed = false;
	LastLandingVerticalSpeed = 0.f;

	UpdateGait();
}

void AConcreteAscentCharacter::UpdateTraversalWarpTargets(const FTraversalCheckResult& TraversalResult)
{
	if (!MotionWarpingComponent)
		return;

	if (TraversalResult.ActionType == ETraversalAction::LedgeGrab)
	{
		MotionWarpingComponent->RemoveWarpTarget(FrontLedgeWarpTargetName);
		MotionWarpingComponent->RemoveWarpTarget(BackLedgeWarpTargetName);
		MotionWarpingComponent->RemoveWarpTarget(BackFloorWarpTargetName);
		return;
	}

	FVector TraversalForward = TraversalResult.BackLedgeLocation - TraversalResult.FrontLedgeLocation;

	TraversalForward.Z = 0.f;
	if (!TraversalForward.Normalize())
	{
		TraversalForward = -TraversalResult.FrontLedgeNormal.GetSafeNormal();
		TraversalForward.Z = 0.f;
		TraversalForward.Normalize();
	}

	FVector ActorForward = GetActorForwardVector();
	ActorForward.Z = 0.f;
	ActorForward.Normalize();

	if (!ActorForward.IsNearlyZero() && FVector::DotProduct(TraversalForward, ActorForward) < 0.f)
		TraversalForward *= -1.f;

	if (TraversalForward.IsNearlyZero())
		TraversalForward = ActorForward;

	if (TraversalForward.IsNearlyZero())
		TraversalForward = FVector::ForwardVector;

	if (TraversalResult.bHasFrontLedge)
	{
		FVector FrontTargetLocation = TraversalResult.FrontLedgeLocation + FVector(0.f, 0.f, 0.5f);

		FVector FrontOutward = TraversalResult.FrontLedgeNormal;
		FrontOutward.Z = 0.f;

		if (!FrontOutward.Normalize())
			FrontOutward = -TraversalForward;

		FrontTargetLocation += FrontOutward * FrontLedgeOutwardOffset;

		MotionWarpingComponent->AddOrUpdateWarpTargetFromTransform(FrontLedgeWarpTargetName, MakeTraversalWarpTransformFromForward(FrontTargetLocation, TraversalForward));
	}
	else
		MotionWarpingComponent->RemoveWarpTarget(FrontLedgeWarpTargetName);

	const bool bNeedsBackLedge = TraversalResult.ActionType == ETraversalAction::Hurdle || TraversalResult.ActionType == ETraversalAction::Vault;

	FVector BackLedgeTargetLocation = TraversalResult.BackLedgeLocation + FVector(0.f, 0.f, 0.5f);

	if (bNeedsBackLedge && TraversalResult.bHasBackLedge)
		MotionWarpingComponent->AddOrUpdateWarpTargetFromTransform(BackLedgeWarpTargetName, MakeTraversalWarpTransformFromForward(BackLedgeTargetLocation, TraversalForward));
	else
		MotionWarpingComponent->RemoveWarpTarget(BackLedgeWarpTargetName);

	const bool bNeedsBackFloor = TraversalResult.ActionType == ETraversalAction::Hurdle;

	if (bNeedsBackFloor && TraversalResult.bHasBackFloor)
	{
		FVector BackFloorTargetLocation = TraversalResult.BackFloorLocation;

		float AnimatedDistanceToBackLedge = 0.f;
		float AnimatedDistanceToBackFloor = 0.f;

		const bool bGotBackLedgeDistance = BP_GetDistanceFromLedgeAtWarpEnd(TraversalResult.ChosenMontage, BackLedgeWarpTargetName, AnimatedDistanceToBackLedge);
		const bool bGotBackFloorDistance = BP_GetDistanceFromLedgeAtWarpEnd(TraversalResult.ChosenMontage, BackFloorWarpTargetName, AnimatedDistanceToBackFloor);

		if (bGotBackLedgeDistance && bGotBackFloorDistance)
		{
			const float ExtraFloorDistance = FMath::Abs(AnimatedDistanceToBackFloor - AnimatedDistanceToBackLedge);

			FVector BackFloorDirection = TraversalResult.BackLedgeNormal;
			BackFloorDirection.Z = 0.f;

			if (!BackFloorDirection.Normalize())
				BackFloorDirection = TraversalForward;

			const FVector BackFloorXYLocation =
				TraversalResult.BackLedgeLocation
				+ BackFloorDirection * ExtraFloorDistance;

			BackFloorTargetLocation.X = BackFloorXYLocation.X;
			BackFloorTargetLocation.Y = BackFloorXYLocation.Y;
			BackFloorTargetLocation.Z = TraversalResult.BackFloorLocation.Z;
		}

		MotionWarpingComponent->AddOrUpdateWarpTargetFromTransform(BackFloorWarpTargetName, MakeTraversalWarpTransformFromForward(BackFloorTargetLocation, TraversalForward));
	}
	else
		MotionWarpingComponent->RemoveWarpTarget(BackFloorWarpTargetName);
}

float AConcreteAscentCharacter::PlayTraversalMontage(UAnimMontage* Montage, float PlayRate, float StartTime)
{
	if (!Montage)
		return 0.f;

	USkeletalMeshComponent* MeshComp = GetMesh();
	if (!MeshComp)
		return 0.f;

	UAnimInstance* AnimInstance = MeshComp->GetAnimInstance();
	if (!AnimInstance)
		return 0.f;

	const float MontageLength = AnimInstance->Montage_Play(Montage, PlayRate, EMontagePlayReturnType::MontageLength, StartTime);

	if (MontageLength <= 0.f)
		return 0.f;

	// 파쿠르 몽타주가 끝나면 캐릭터 상태와 MovementMode를 복구한다.
	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &AConcreteAscentCharacter::OnTraversalMontageEnded);
	AnimInstance->Montage_SetEndDelegate(EndDelegate, Montage);

	return MontageLength;
}