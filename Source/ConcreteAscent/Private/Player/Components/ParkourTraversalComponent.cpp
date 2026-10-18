// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/Components/ParkourTraversalComponent.h"
#include "Player/ConcreteAscentCharacter.h"
#include "Objects/Obstacle/LedgeObstacle.h"
#include "Objects/Obstacle/ParkourObstacleBase.h"
#include "Data/ParkourMotionData.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimTypes.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Components/SkeletalMeshComponent.h"
#include "TimerManager.h"

UParkourTraversalComponent::UParkourTraversalComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UParkourTraversalComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<AConcreteAscentCharacter>(GetOwner());
}

void UParkourTraversalComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsLedgeMoving || !OwnerCharacter || !CurrentLedge)
		return;

	LedgeMoveElapsedTime += DeltaTime;

	const float Alpha = LedgeMoveDuration > KINDA_SMALL_NUMBER ? FMath::Clamp(LedgeMoveElapsedTime / LedgeMoveDuration, 0.f, 1.f) : 1.f;
	CurrentLedgeOffset = FMath::Lerp(LedgeMoveStartOffset, LedgeMoveTargetOffset, Alpha);

	const FVector NewLedgePoint = CurrentLedgeCenter + CurrentLedgeRight * CurrentLedgeOffset;
	OwnerCharacter->SetHangLocationFromLedgePoint(NewLedgePoint, CurrentLedgeNormal);

	if (Alpha >= 1.f)
	{
		CurrentLedgeOffset = LedgeMoveTargetOffset;
		FinishLedgeMove();
	}
}

AParkourObstacleBase* UParkourTraversalComponent::DetectObstacle()
{
	if (!OwnerCharacter || !GetWorld())
		return nullptr;

	const FVector ActorLocation = OwnerCharacter->GetActorLocation();
	const FVector Forward = OwnerCharacter->GetActorForwardVector();

	const FVector Start = ActorLocation + FVector(0.f, 0.f, TraceOriginZOffset);
	const FVector End = Start + Forward * TraceForwardDistance;

	FHitResult Hit;
	const bool bHit = CapsuleSweep(Start, End, TraversableTraceChannel, Hit);

	if (!bHit || !Hit.bBlockingHit || !Hit.GetActor())
	{
		LastObstacleHit = FHitResult();
		return nullptr;
	}

	AParkourObstacleBase* Obstacle = Cast<AParkourObstacleBase>(Hit.GetActor());
	if (!Obstacle)
	{
		LastObstacleHit = FHitResult();
		return nullptr;
	}

	// 이후 ledge 계산에서 충돌 지점 정보가 필요하므로 마지막 Hit 정보를 함께 저장한다.
	LastObstacleHit = Hit;

	return Obstacle;
}

FTraversalChooserInputs UParkourTraversalComponent::EvaluateTraversal(AParkourObstacleBase* ObstacleBase)
{
	// 장애물과 캐릭터 상태를 기반으로 Chooser에 전달할 입력값을 만든다.
	CurrentTraversalResult = BuildTraversalCheckResult(ObstacleBase);
	CurrentTraversalInputs = MakeChooserInputsFromCheckResult(CurrentTraversalResult);

	return CurrentTraversalInputs;
}

bool UParkourTraversalComponent::StartTraversal(FTraversalCheckResult& OutResult)
{
	if (!OwnerCharacter)
		return false;

	OutResult = FTraversalCheckResult();

	AParkourObstacleBase* Obstacle = DetectObstacle();
	if (!Obstacle)
		return false;

	// LedgeObstacle은 지상 파쿠르 흐름에서 처리하지 않는다.
	// 지상 Space는 일반 점프로 보내고,
	// 공중 Space에서 TryAirLedgeGrab()으로만 매달리기를 처리한다.
	if (Obstacle->IsA<ALedgeObstacle>())
		return false;

	CurrentTraversalInputs = EvaluateTraversal(Obstacle);

	if (!CurrentTraversalInputs.bHasFrontLedge)
	{
		UE_LOG(LogTemp, Warning, TEXT("Traversal failed: No front ledge."));
		return false;
	}

	TArray<UAnimMontage*> ValidMontages = BuildValidTraversalMontages(CurrentTraversalInputs, CurrentTraversalOutputs);

	CurrentTraversalInputs.ActionType = CurrentTraversalOutputs.ActionType;
	CurrentTraversalResult.ActionType = CurrentTraversalOutputs.ActionType;

	if (CurrentTraversalOutputs.ActionType == ETraversalAction::None)
	{
		UE_LOG(LogTemp, Warning, TEXT("Traversal failed: ActionType is None."));
		return false;
	}

	if (ValidMontages.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("Traversal failed: No valid montages."));
		return false;
	}

	BP_UpdatePoseSearchPlayerActor(OwnerCharacter);

	UAnimMontage* SelectedMontage = ValidMontages[0];

	float StartTime = 0.f;
	FindPoseSearchBranchInEndTime(SelectedMontage, StartTime);

	CurrentTraversalResult.ChosenMontage = SelectedMontage;
	CurrentTraversalResult.StartTime = StartTime;
	CurrentTraversalResult.PlayRate = 1.f;

	OutResult = CurrentTraversalResult;
	return true;
}

bool UParkourTraversalComponent::TryAirLedgeGrab(FTraversalCheckResult& OutResult)
{
	OutResult = FTraversalCheckResult();

	if (!OwnerCharacter)
		return false;

	const UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement();
	if (!MovementComp || !MovementComp->IsFalling())
		return false;

	AParkourObstacleBase* Obstacle = DetectObstacle();
	if (!Obstacle)
	{
		UE_LOG(LogTemp, Warning, TEXT("AirLedgeGrab failed: No obstacle."));
		return false;
	}

	if (!Obstacle->IsA<ALedgeObstacle>())
	{
		UE_LOG(LogTemp, Warning, TEXT("AirLedgeGrab failed: Obstacle is not LedgeObstacle."));
		return false;
	}

	CurrentTraversalInputs = EvaluateTraversal(Obstacle);

	if (!CurrentTraversalInputs.bHasFrontLedge)
	{
		UE_LOG(LogTemp, Warning, TEXT("AirLedgeGrab failed: No front ledge."));
		return false;
	}

	CurrentLedge = Cast<ALedgeObstacle>(Obstacle);
	if (!CurrentLedge)
		return false;

	FVector SegmentCenter;
	FVector SegmentRight;
	float SegmentHalfLength = 0.f;

	if (!CurrentLedge->GetLedgeMoveSegment(CurrentTraversalResult.FrontLedgeNormal, SegmentCenter, SegmentRight, SegmentHalfLength))
	{
		UE_LOG(LogTemp, Warning, TEXT("AirLedgeGrab failed: Invalid ledge move segment."));
		return false;
	}

	// 입력 X 양수가 캐릭터의 오른쪽 방향과 최대한 맞도록 이동축을 정렬한다.
	FVector CharacterRight = OwnerCharacter->GetActorRightVector();
	CharacterRight.Z = 0.f;
	CharacterRight.Normalize();
	if (!CharacterRight.IsNearlyZero() && FVector::DotProduct(SegmentRight, CharacterRight) < 0.f)
		SegmentRight *= -1.f;

	const UCapsuleComponent* CapsuleComp = OwnerCharacter->GetCapsuleComponent();
	const float CapsuleRadius = CapsuleComp ? CapsuleComp->GetScaledCapsuleRadius() : 34.f;

	CurrentLedgeCenter = SegmentCenter;
	CurrentLedgeRight = SegmentRight.GetSafeNormal();
	CurrentLedgeNormal = CurrentTraversalResult.FrontLedgeNormal.GetSafeNormal();

	CurrentLedgeMinOffset = -SegmentHalfLength + CapsuleRadius + LedgeEdgePadding;
	CurrentLedgeMaxOffset = SegmentHalfLength - CapsuleRadius - LedgeEdgePadding;

	if (CurrentLedgeMinOffset > CurrentLedgeMaxOffset)
	{
		CurrentLedgeMinOffset = 0.f;
		CurrentLedgeMaxOffset = 0.f;
	}

	CurrentLedgeOffset = FVector::DotProduct(CurrentTraversalResult.FrontLedgeLocation - CurrentLedgeCenter, CurrentLedgeRight);
	CurrentLedgeOffset = FMath::Clamp(CurrentLedgeOffset, CurrentLedgeMinOffset, CurrentLedgeMaxOffset);

	const FVector ClampedLedgePoint = CurrentLedgeCenter + CurrentLedgeRight * CurrentLedgeOffset;

	CurrentTraversalInputs.ActionType = ETraversalAction::LedgeGrab;
	CurrentTraversalOutputs = FTraversalChooserOutputs();
	CurrentTraversalOutputs.ActionType = ETraversalAction::LedgeGrab;

	CurrentTraversalResult.ActionType = ETraversalAction::LedgeGrab;
	CurrentTraversalResult.FrontLedgeLocation = ClampedLedgePoint;
	CurrentTraversalResult.FrontLedgeNormal = CurrentLedgeNormal;
	CurrentTraversalResult.ChosenMontage = nullptr;
	CurrentTraversalResult.StartTime = 0.f;
	CurrentTraversalResult.PlayRate = 1.f;

	OutResult = CurrentTraversalResult;

	return true;
}

void UParkourTraversalComponent::MoveAlongLedge(float Direction)
{
	if (!OwnerCharacter || !CurrentLedge)
		return;

	// 좌우 이동 중이거나 기어오르는 중이면 추가 입력 무시
	if (bIsLedgeMoving || bIsLedgeClimbing)
		return;

	if (FMath::IsNearlyZero(Direction, 0.15f))
		return;

	const float DirectionSign = Direction > 0.f ? 1.f : -1.f;

	const float TargetOffset = FMath::Clamp(CurrentLedgeOffset + DirectionSign * LedgeMoveStepDistance, CurrentLedgeMinOffset, CurrentLedgeMaxOffset);

	// 난간 끝이면 이동하지 않음
	if (FMath::IsNearlyEqual(TargetOffset, CurrentLedgeOffset, 0.1f))
		return;

	const ETraversalAction MoveAction = DirectionSign < 0.f ? ETraversalAction::LedgeMoveLeft : ETraversalAction::LedgeMoveRight;

	const float MontageLength = PlayLedgeMontage(MoveAction);

	LedgeMoveStartOffset = CurrentLedgeOffset;
	LedgeMoveTargetOffset = TargetOffset;
	LedgeMoveElapsedTime = 0.f;

	LedgeMoveDuration = MontageLength > 0.f ? MontageLength : LedgeMoveDefaultDuration;
	LedgeMoveDuration = FMath::Max(LedgeMoveDuration, 0.05f);

	bIsLedgeMoving = true;
	SetComponentTickEnabled(true);
}

void UParkourTraversalComponent::ClimbFromLedge()
{
	if (!OwnerCharacter || !CurrentLedge)
		return;

	if (bIsLedgeMoving || bIsLedgeClimbing)
		return;

	FVector StandLocation;
	FRotator StandRotation;

	if (!CanClimbFromLedge(StandLocation, StandRotation))
	{
		UE_LOG(LogTemp, Warning, TEXT("ClimbFromLedge failed: No room above ledge."));
		return;
	}

	// 좌우 이동 몽타주가 남아 있으면 먼저 끊는다.
	StopLedgeMontages(0.0f);

	// 이전 Climb 타이머가 남아 있으면 제거한다.
	if (GetWorld())
		GetWorld()->GetTimerManager().ClearTimer(LedgeClimbFinishTimerHandle);

	// Climb 중에는 현재 LedgeObstacle의 Blocking Collision에 막힐 수 있으므로 충돌을 임시로 무시한다.
	SetCurrentLedgeCollisionIgnored(true);

	const FVector CurrentLedgePoint =
		CurrentLedgeCenter + CurrentLedgeRight * CurrentLedgeOffset;

	// Climb 몽타주의 시작 손 위치 보정.
	OwnerCharacter->SetClimbStartLocationFromLedgePoint(CurrentLedgePoint, CurrentLedgeNormal);

	// Climb 중 입력 방지.
	OwnerCharacter->EnterLedgeClimbState();

	OwnerCharacter->SetClimbStandWarpTarget(StandLocation, StandRotation);
	PendingClimbStandLocation = StandLocation;
	PendingClimbStandRotation = StandRotation;

	bIsLedgeClimbing = true;
	bIsLedgeMoving = false;
	SetComponentTickEnabled(false);

	const float MontageLength = PlayLedgeMontage(ETraversalAction::LedgeClimbUp);

	// 정상적으로 몽타주가 재생된 경우에는 Montage End Delegate에서 FinishLedgeClimb을 호출한다.
	// 몽타주 재생 실패 시에만 fallback 타이머를 건다.
	if (MontageLength <= 0.f && GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(LedgeClimbFinishTimerHandle, this, &UParkourTraversalComponent::FinishLedgeClimb, LedgeClimbDefaultDuration, false);
	}
}

void UParkourTraversalComponent::DropFromLedge()
{
	if (!OwnerCharacter)
		return;

	if (GetWorld())
		GetWorld()->GetTimerManager().ClearTimer(LedgeClimbFinishTimerHandle);

	// 좌우 이동 / 기어오르기 중단
	bIsLedgeMoving = false;
	bIsLedgeClimbing = false;
	SetComponentTickEnabled(false);

	LedgeMoveStartOffset = 0.f;
	LedgeMoveTargetOffset = 0.f;
	LedgeMoveElapsedTime = 0.f;
	LedgeMoveDuration = 0.f;

	PendingClimbStandLocation = FVector::ZeroVector;
	PendingClimbStandRotation = FRotator::ZeroRotator;

	// 남아 있는 좌우 이동 / 기어오르기 몽타주 중단
	StopLedgeMontages(0.05f);

	// Climb용 Motion Warping Target 제거
	OwnerCharacter->ClearClimbStandWarpTarget();

	// 충돌 복구
	SetCurrentLedgeCollisionIgnored(false);

	OwnerCharacter->ExitHangingToFalling();

	ResetLedgeRuntimeState();
}

TArray<UAnimMontage*> UParkourTraversalComponent::BuildValidTraversalMontages(const FTraversalChooserInputs& Inputs, FTraversalChooserOutputs& OutOutputs)
{
	TArray<UAnimMontage*> Result;
	OutOutputs = FTraversalChooserOutputs();

	// Chooser 평가는 블루프린트에서 수행하고, C++은 결과만 받아서 사용한다.
	BP_EvaluateTraversalChooser(Inputs, OutOutputs, Result);

	return Result;
}

bool UParkourTraversalComponent::FindPoseSearchBranchInEndTime(const UAnimMontage* Montage, float& OutEndTime) const
{
	OutEndTime = 0.f;

	if (!Montage)
		return false;

	bool bFound = false;
	float EarliestStartTime = TNumericLimits<float>::Max();

	for (const FAnimNotifyEvent& NotifyEvent : Montage->Notifies)
	{
		const UAnimNotifyState* NotifyState = NotifyEvent.NotifyStateClass;
		if (!NotifyState)
			continue;

		const FString NotifyClassName = NotifyState->GetClass()->GetName();
		if (!NotifyClassName.Contains(TEXT("PoseSearchBranchIn")))
			continue;

		const float StartTime = NotifyEvent.GetTriggerTime();
		const float EndTime = NotifyEvent.GetEndTriggerTime();

		if (EndTime <= StartTime)
		{
			continue;
		}

		// 여러 BranchIn 구간이 있을 경우 가장 앞에 있는 구간을 기준으로 사용한다.
		if (StartTime < EarliestStartTime)
		{
			EarliestStartTime = StartTime;
			OutEndTime = EndTime;
			bFound = true;
		}
	}

	return bFound;
}

float UParkourTraversalComponent::PlayLedgeMontage(ETraversalAction Action)
{
	if (!OwnerCharacter || !MotionData)
		return 0.f;

	UAnimMontage* Montage = nullptr;

	switch (Action)
	{
	case ETraversalAction::LedgeGrab:
		Montage = MotionData->GetLedgeGrabMontage();
		break;

	case ETraversalAction::LedgeClimbUp:
		Montage = MotionData->GetLedgeClimbUpMontage();
		break;

	case ETraversalAction::LedgeMoveLeft:
		Montage = MotionData->GetLedgeMoveMontage(-1.f);
		break;

	case ETraversalAction::LedgeMoveRight:
		Montage = MotionData->GetLedgeMoveMontage(1.f);
		break;

	default:
		break;
	}

	if (!Montage)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayLedgeMontage failed: Montage is null."));
		return 0.f;
	}

	USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh();
	if (!MeshComp)
	{
		return 0.f;
	}

	UAnimInstance* AnimInstance = MeshComp->GetAnimInstance();
	if (!AnimInstance)
	{
		return 0.f;
	}

	const float StopBlendTime = Action == ETraversalAction::LedgeClimbUp ? 0.f : 0.05f;

	AnimInstance->Montage_Stop(StopBlendTime, nullptr);

	const float MontageLength = AnimInstance->Montage_Play(Montage, 1.f, EMontagePlayReturnType::MontageLength, 0.f);

	if (MontageLength > 0.f && Action == ETraversalAction::LedgeClimbUp)
	{
		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &UParkourTraversalComponent::OnLedgeClimbMontageEnded);
		AnimInstance->Montage_SetEndDelegate(EndDelegate, Montage);
	}

	return MontageLength;
}

void UParkourTraversalComponent::FinishLedgeMove()
{
	if (!OwnerCharacter || !CurrentLedge)
	{
		bIsLedgeMoving = false;
		SetComponentTickEnabled(false);
		return;
	}

	bIsLedgeMoving = false;
	LedgeMoveElapsedTime = 0.f;
	LedgeMoveDuration = 0.f;

	CurrentLedgeOffset = LedgeMoveTargetOffset;

	const FVector FinalLedgePoint = CurrentLedgeCenter + CurrentLedgeRight * CurrentLedgeOffset;
	OwnerCharacter->SetHangLocationFromLedgePoint(FinalLedgePoint, CurrentLedgeNormal);

	SetComponentTickEnabled(false);
}

bool UParkourTraversalComponent::CanClimbFromLedge(FVector& OutStandLocation, FRotator& OutStandRotation) const
{
	OutStandLocation = FVector::ZeroVector;
	OutStandRotation = FRotator::ZeroRotator;

	if (!OwnerCharacter || !CurrentLedge || !GetWorld())
		return false;

	const UCapsuleComponent* CapsuleComp = OwnerCharacter->GetCapsuleComponent();
	if (!CapsuleComp)
		return false;

	const float CapsuleRadius = CapsuleComp->GetScaledCapsuleRadius();
	const float CapsuleHalfHeight = CapsuleComp->GetScaledCapsuleHalfHeight();

	FVector Outward = CurrentLedgeNormal;
	Outward.Z = 0.f;

	if (!Outward.Normalize())
		return false;

	const FVector Inward = -Outward;

	const FVector CurrentLedgePoint =
		CurrentLedgeCenter + CurrentLedgeRight * CurrentLedgeOffset;

	// 일단 난간 안쪽 XY 위치를 계산한다.
	const FVector DesiredStandXY =
		CurrentLedgePoint
		+ Inward * (CapsuleRadius + LedgeClimbStandForwardOffset);

	// 실제 바닥 높이를 찾기 위해 위에서 아래로 트레이스한다.
	const FVector FloorTraceStart =
		DesiredStandXY + FVector(0.f, 0.f, CapsuleHalfHeight + FloorCheckExtraDistance);

	const FVector FloorTraceEnd =
		DesiredStandXY - FVector(0.f, 0.f, FloorCheckExtraDistance * 2.f);

	FCollisionQueryParams Params(SCENE_QUERY_STAT(LedgeClimbFloorCheck), false);
	Params.AddIgnoredActor(OwnerCharacter);

	// CurrentLedge 자체를 무시하면 바닥을 못 잡을 수 있으므로 여기서는 무시하지 않는다.
	FHitResult FloorHit;
	const bool bFloorHit = GetWorld()->LineTraceSingleByChannel(FloorHit, FloorTraceStart, FloorTraceEnd, RoomTraceChannel, Params);

	if (!bFloorHit || !FloorHit.bBlockingHit)
	{
		UE_LOG(LogTemp, Warning, TEXT("CanClimbFromLedge failed: No floor hit."));
		return false;
	}

	OutStandLocation = FVector(DesiredStandXY.X, DesiredStandXY.Y, FloorHit.ImpactPoint.Z + CapsuleHalfHeight + LedgeClimbStandZOffset);
	OutStandRotation = Inward.Rotation();

	// 서는 위치에 캡슐이 들어갈 공간이 있는지 검사한다.
	FCollisionQueryParams OverlapParams(SCENE_QUERY_STAT(LedgeClimbRoomCheck), false);
	OverlapParams.AddIgnoredActor(OwnerCharacter);

	const FCollisionShape CapsuleShape = FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight);
	const bool bBlocked = GetWorld()->OverlapBlockingTestByChannel(OutStandLocation, FQuat::Identity, RoomTraceChannel, CapsuleShape, OverlapParams);

	return !bBlocked;
}

void UParkourTraversalComponent::FinishLedgeClimb()
{
	if (!OwnerCharacter)
		return;

	if (GetWorld())
		GetWorld()->GetTimerManager().ClearTimer(LedgeClimbFinishTimerHandle);

	bIsLedgeClimbing = false;
	bIsLedgeMoving = false;
	SetComponentTickEnabled(false);

	OwnerCharacter->ExitHangingToStanding(PendingClimbStandLocation, PendingClimbStandRotation);

	// 충돌 복구.
	SetCurrentLedgeCollisionIgnored(false);

	ResetLedgeRuntimeState();
}

void UParkourTraversalComponent::OnLedgeClimbMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!bIsLedgeClimbing)
		return;

	if (bInterrupted)
	{
		bIsLedgeClimbing = false;
		bIsLedgeMoving = false;
		SetComponentTickEnabled(false);

		SetCurrentLedgeCollisionIgnored(false);

		if (OwnerCharacter)
			OwnerCharacter->ExitHangingToFalling();

		ResetLedgeRuntimeState();
		return;
	}

	FinishLedgeClimb();
}

void UParkourTraversalComponent::ResetLedgeRuntimeState()
{
	if (bIsCurrentLedgeCollisionIgnored)
		SetCurrentLedgeCollisionIgnored(false);

	CurrentLedge = nullptr;

	CurrentLedgeCenter = FVector::ZeroVector;
	CurrentLedgeRight = FVector::RightVector;
	CurrentLedgeNormal = FVector::ForwardVector;

	CurrentLedgeOffset = 0.f;
	CurrentLedgeMinOffset = 0.f;
	CurrentLedgeMaxOffset = 0.f;

	LedgeMoveStartOffset = 0.f;
	LedgeMoveTargetOffset = 0.f;
	LedgeMoveElapsedTime = 0.f;
	LedgeMoveDuration = 0.f;

	PendingClimbStandLocation = FVector::ZeroVector;
	PendingClimbStandRotation = FRotator::ZeroRotator;

	bIsLedgeMoving = false;
	bIsLedgeClimbing = false;
	bIsCurrentLedgeCollisionIgnored = false;

	SetComponentTickEnabled(false);
}

void UParkourTraversalComponent::StopLedgeMontages(float BlendOutTime)
{
	if (!OwnerCharacter)
		return;

	USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh();
	if (!MeshComp)
		return;

	UAnimInstance* AnimInstance = MeshComp->GetAnimInstance();
	if (!AnimInstance)
		return;

	AnimInstance->Montage_Stop(BlendOutTime, nullptr);
}

void UParkourTraversalComponent::SetCurrentLedgeCollisionIgnored(bool bIgnore)
{
	if (!OwnerCharacter)
		return;

	UCapsuleComponent* CapsuleComp = OwnerCharacter->GetCapsuleComponent();
	if (!CapsuleComp)
		return;

	if (CurrentLedge)
		CapsuleComp->IgnoreActorWhenMoving(CurrentLedge, bIgnore);

	if (UPrimitiveComponent* HitComponent = CurrentTraversalResult.HitComponent.Get())
		CapsuleComp->IgnoreComponentWhenMoving(HitComponent, bIgnore);

	bIsCurrentLedgeCollisionIgnored = bIgnore;
}

bool UParkourTraversalComponent::CapsuleSweep(const FVector& Start, const FVector& End, ECollisionChannel Channel, FHitResult& OutHit) const
{
	if (!OwnerCharacter || !GetWorld())
		return false;

	const UCapsuleComponent* CapsuleComp = OwnerCharacter->GetCapsuleComponent();
	if (!CapsuleComp)
		return false;

	const float Radius = CapsuleComp->GetScaledCapsuleRadius();
	const float HalfHeight = CapsuleComp->GetScaledCapsuleHalfHeight();

	const FCollisionShape Shape = FCollisionShape::MakeCapsule(Radius, HalfHeight);

	FCollisionQueryParams Params(SCENE_QUERY_STAT(ParkourCapsuleSweep), false);
	Params.AddIgnoredActor(OwnerCharacter);

	return GetWorld()->SweepSingleByChannel(OutHit, Start, End, FQuat::Identity, Channel, Shape, Params);
}

bool UParkourTraversalComponent::HasCapsuleRoom(const FVector& Start, const FVector& End, ECollisionChannel Channel, FHitResult& OutHit) const
{
	const bool bHit = CapsuleSweep(Start, End, Channel, OutHit);

	if (!bHit)
		return true;

	return !(OutHit.bBlockingHit || OutHit.bStartPenetrating);
}

FTraversalCheckResult UParkourTraversalComponent::BuildTraversalCheckResult(AParkourObstacleBase* ObstacleBase)
{
	FTraversalCheckResult Result;

	if (!OwnerCharacter || !ObstacleBase)
		return Result;

	const UCapsuleComponent* CapsuleComp = OwnerCharacter->GetCapsuleComponent();
	if (!CapsuleComp)
		return Result;

	const FVector ActorLocation = OwnerCharacter->GetActorLocation();
	const float CapsuleRadius = CapsuleComp->GetScaledCapsuleRadius();
	const float CapsuleHalfHeight = CapsuleComp->GetScaledCapsuleHalfHeight();

	FVector FrontLedgeLocation;
	FVector FrontLedgeNormal;
	FVector BackLedgeLocation;
	FVector BackLedgeNormal;
	float ObstacleHeight = 0.f;
	float ObstacleDepth = 0.f;

	const bool bGotLedgeData = ObstacleBase->GetTraversalLedgeData(
		LastObstacleHit,
		ActorLocation,
		FrontLedgeLocation,
		FrontLedgeNormal,
		BackLedgeLocation,
		BackLedgeNormal,
		ObstacleHeight,
		ObstacleDepth
	);

	if (!bGotLedgeData)
		return Result;

	Result.bHasFrontLedge = true;
	Result.bHasBackLedge = true;

	Result.FrontLedgeLocation = FrontLedgeLocation;
	Result.FrontLedgeNormal = FrontLedgeNormal.GetSafeNormal();

	Result.BackLedgeLocation = BackLedgeLocation;
	Result.BackLedgeNormal = BackLedgeNormal.GetSafeNormal();

	const FVector CapsuleLocation = CapsuleComp->GetComponentLocation();
	const float CapsuleBottomZ = CapsuleLocation.Z - CapsuleHalfHeight;

	Result.ObstacleHeight = FMath::Max(0.f, Result.FrontLedgeLocation.Z - CapsuleBottomZ);
	Result.ObstacleDepth = ObstacleDepth;
	Result.HitComponent = LastObstacleHit.GetComponent();

	const bool bIsLedgeObstacle = ObstacleBase->IsA<ALedgeObstacle>();

	if (bIsLedgeObstacle)
	{
		CurrentLedge = Cast<ALedgeObstacle>(ObstacleBase);

		Result.ActionType = ETraversalAction::LedgeGrab;

		// LedgeObstacle은 매달리기 전용이므로 앞 난간만 있으면 된다.
		Result.bHasFrontLedge = true;
		Result.bHasBackLedge = false;
		Result.bHasBackFloor = false;
		Result.BackLedgeHeight = 0.f;

		return Result;
	}

	// 여기부터는 일반 Vault / Hurdle / Mantle용 공간 검사.

	const FVector FrontRoomLocation =
		Result.FrontLedgeLocation
		+ Result.FrontLedgeNormal * (CapsuleRadius + LedgeRoomPadding)
		+ FVector(0.f, 0.f, CapsuleHalfHeight + LedgeRoomPadding);

	FHitResult FrontRoomHit;
	const bool bHasFrontRoom = HasCapsuleRoom(ActorLocation, FrontRoomLocation, RoomTraceChannel, FrontRoomHit);

	if (!bHasFrontRoom)
	{
		Result.bHasFrontLedge = false;
		Result.bHasBackLedge = false;
		Result.bHasBackFloor = false;
		return Result;
	}

	const FVector BackRoomLocation =
		Result.BackLedgeLocation
		+ Result.BackLedgeNormal * (CapsuleRadius + LedgeRoomPadding)
		+ FVector(0.f, 0.f, CapsuleHalfHeight + LedgeRoomPadding);

	FHitResult TopSweepHit;
	const bool bHasTopRoom = HasCapsuleRoom(FrontRoomLocation,BackRoomLocation,RoomTraceChannel,TopSweepHit);

	if (!bHasTopRoom)
	{
		Result.bHasBackLedge = false;
		Result.bHasBackFloor = false;
		return Result;
	}

	const FVector FloorTraceStart =BackRoomLocation + FVector(0.f, 0.f, FloorCheckExtraDistance);

	const float FloorCheckDistance =
		CapsuleHalfHeight
		+ Result.ObstacleHeight * 0.5f
		+ FloorCheckExtraDistance;

	const FVector FloorTraceEnd = FloorTraceStart - FVector(0.f, 0.f, FloorCheckDistance);

	FHitResult FloorHit;
	const bool bFloorHit = CapsuleSweep(FloorTraceStart, FloorTraceEnd, RoomTraceChannel, FloorHit);

	if (!bFloorHit || !FloorHit.bBlockingHit)
	{
		Result.bHasBackFloor = false;
		return Result;
	}

	Result.bHasBackFloor = true;
	Result.BackFloorLocation = FloorHit.ImpactPoint;
	Result.BackLedgeHeight = FMath::Abs(Result.BackLedgeLocation.Z - FloorHit.ImpactPoint.Z);

	return Result;
}

FTraversalChooserInputs UParkourTraversalComponent::MakeChooserInputsFromCheckResult(const FTraversalCheckResult& CheckResult) const
{
	FTraversalChooserInputs Inputs;

	Inputs.ActionType = ETraversalAction::None;

	Inputs.bHasFrontLedge = CheckResult.bHasFrontLedge;
	Inputs.bHasBackLedge = CheckResult.bHasBackLedge;
	Inputs.bHasBackFloor = CheckResult.bHasBackFloor;

	Inputs.ObstacleHeight = CheckResult.ObstacleHeight;
	Inputs.ObstacleDepth = CheckResult.ObstacleDepth;
	Inputs.BackLedgeHeight = CheckResult.BackLedgeHeight;

	if (OwnerCharacter)
	{
		Inputs.Speed = OwnerCharacter->GetVelocity().Size2D();

		if (const UCharacterMovementComponent* MoveComp = OwnerCharacter->GetCharacterMovement())
			Inputs.MovementMode = MoveComp->MovementMode;

		Inputs.Gait = OwnerCharacter->GetGait();
	}

	return Inputs;
}
