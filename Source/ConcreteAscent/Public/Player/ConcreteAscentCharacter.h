// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Types/ConcreteAscentTypes.h"
#include "ConcreteAscentCharacter.generated.h"

class AConcreteAscentPlayerController;
class UInputAction;
class UMotionWarpingComponent;
class UParkourTraversalComponent;
class USpringArmComponent;
class UCameraComponent;
class UAnimMontage;

UCLASS()
class CONCRETEASCENT_API AConcreteAscentCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AConcreteAscentCharacter();

protected:
	// ACharacter interface
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void Landed(const FHitResult& Hit) override;

protected:
	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ascent|Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ascent|Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ascent|Traversal")
	TObjectPtr<UMotionWarpingComponent> MotionWarpingComponent;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Ascent|Traversal")
	TObjectPtr<UParkourTraversalComponent> ParkourTraversalComponent;

protected:
	// Input actions
	UPROPERTY(EditDefaultsOnly, Category = "Ascent|Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = "Ascent|Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, Category = "Ascent|Input")
	TObjectPtr<UInputAction> JumpInputAction;

	UPROPERTY(EditDefaultsOnly, Category = "Ascent|Input")
	TObjectPtr<UInputAction> WalkInputAction;

	UPROPERTY(EditDefaultsOnly, Category = "Ascent|Input")
	TObjectPtr<UInputAction> SprintInputAction;

	UPROPERTY(EditDefaultsOnly, Category = "Ascent|Input")
	TObjectPtr<UInputAction> PauseInputAction;

protected:
	// Movement state
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ascent|Movement")
	EGait Gait = EGait::Walk;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ascent|Movement")
	bool bCanMove = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ascent|Movement")
	bool bWalk = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ascent|Movement")
	bool bSprint = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ascent|Movement")
	bool bIsHanging = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ascent|Movement")
	bool bJustLanded = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ascent|Movement")
	bool bWasHangingMoveInputPressed = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ascent|Movement")
	bool bWasHangingVerticalInputPressed = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ascent|Movement")
	float LastLandingVerticalSpeed = 0.f;

protected:
	// Movement settings
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ascent|Movement")
	float WalkMaxSpeed = 200.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ascent|Movement")
	float RunMaxSpeed = 500.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ascent|Movement")
	float SprintMaxSpeed = 700.f;

	// Traversal state
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ascent|Traversal")
	bool bIsTraversing = false;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Ascent|Traversal")
	FTraversalCheckResult CurrentTraversalResult;

	// Traversal settings
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ascent|Traversal|MotionWarping")
	float FrontLedgeOutwardOffset = 8.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ascent|Traversal|Ledge")
	float HangWallGap = 8.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ascent|Traversal|Ledge")
	float HangRootZOffset = 155.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ascent|Traversal|Ledge")
	float ClimbStartWallGap = 8.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ascent|Traversal|Ledge")
	float ClimbStartRootZOffset = 155.f;

	UFUNCTION(BlueprintCallable, Category = "Ascent|Traversal|Ledge")
	void SnapToCurrentLedgeHang();

	UFUNCTION(BlueprintCallable, Category = "Ascent|Traversal|Ledge")
	void EnterHangingState();

public:
	UFUNCTION(BlueprintCallable, Category = "Ascent|Traversal|Ledge")
	void SetHangLocationFromLedgePoint(const FVector& LedgePoint, const FVector& LedgeNormal);

	UFUNCTION(BlueprintCallable, Category = "Ascent|Traversal|Ledge")
	void SetClimbStartLocationFromLedgePoint(const FVector& LedgePoint, const FVector& LedgeNormal);

	UFUNCTION(BlueprintCallable, Category = "Ascent|Traversal|MotionWarping")
	void SetClimbStandWarpTarget(const FVector& StandLocation, const FRotator& StandRotation);

	UFUNCTION(BlueprintCallable, Category = "Ascent|Traversal|MotionWarping")
	void ClearClimbStandWarpTarget();

	UFUNCTION(BlueprintCallable, Category = "Ascent|Traversal|Ledge")
	void EnterLedgeClimbState();

	UFUNCTION(BlueprintCallable, Category = "Ascent|Traversal|Ledge")
	void ExitHangingToFalling();

	UFUNCTION(BlueprintCallable, Category = "Ascent|Traversal|Ledge")
	void ExitHangingToStanding(const FVector& StandLocation, const FRotator& StandRotation);

protected:
	// Timers
	FTimerHandle JustLandedTimerHandle;

	// Movement helpers
	UFUNCTION(BlueprintCallable, Category = "Ascent|Movement")
	void UpdateGait();

	void ClearJustLanded();

	// Traversal helpers
	UFUNCTION(BlueprintCallable, Category = "Ascent|Traversal")
	void BeginTraversal();

	UFUNCTION(BlueprintCallable, Category = "Ascent|Traversal")
	void EndTraversal();

	UFUNCTION()
	void OnTraversalMontageEnded(UAnimMontage* Montage, bool bInterrupted);

protected:
	// Blueprint extension points
	UFUNCTION(BlueprintImplementableEvent, Category = "Traversal|MotionWarping")
	bool BP_GetDistanceFromLedgeAtWarpEnd(UAnimMontage* Montage, FName WarpTargetName, float& OutDistance) const;

	UFUNCTION(BlueprintImplementableEvent, Category = "Ascent|Traversal|Ledge")
	void BP_OnEnterHangingState();

public:
	// Input
	UFUNCTION(BlueprintCallable, Category = "Input")
	void Move(const FInputActionValue& InputValue);

	UFUNCTION(BlueprintCallable, Category = "Input")
	void Look(const FInputActionValue& InputValue);

	UFUNCTION(BlueprintCallable, Category = "Input")
	void ToggleWalk(const FInputActionValue& InputValue);

	UFUNCTION(BlueprintCallable, Category = "Input")
	void StartSprint(const FInputActionValue& InputValue);

	UFUNCTION(BlueprintCallable, Category = "Input")
	void StopSprint(const FInputActionValue& InputValue);

	UFUNCTION(BlueprintCallable, Category = "Input")
	void TogglePauseMenu(const FInputActionValue& InputValue);

	virtual void Jump() override;

public:
	// Character state
	UFUNCTION(BlueprintCallable, Category = "Character")
	void RespawnAt(const FTransform& RespawnTransform);

public:
	// Traversal
	UFUNCTION(BlueprintCallable, Category = "Traversal")
	void UpdateTraversalWarpTargets(const FTraversalCheckResult& TraversalResult);

	UFUNCTION(BlueprintCallable, Category = "Traversal")
	float PlayTraversalMontage(UAnimMontage* Montage, float PlayRate = 1.f, float StartTime = 0.f);

public:
	// Character getters
	UFUNCTION(BlueprintPure, Category = "Character")
	bool CanMove() const { return bCanMove; }

	UFUNCTION(BlueprintPure, Category = "Character")
	bool IsHanging() const { return bIsHanging; }

	UFUNCTION(BlueprintPure, Category = "Character")
	bool IsWalking() const { return bWalk; }

	UFUNCTION(BlueprintPure, Category = "Character")
	bool IsSprinting() const { return bSprint; }

	UFUNCTION(BlueprintPure, Category = "Character")
	bool IsJustLanded() const { return bJustLanded; }

	UFUNCTION(BlueprintPure, Category = "Character")
	bool IsTraversaling() const { return bIsTraversing; }

	UFUNCTION(BlueprintPure, Category = "Character")
	float GetLastLandingVerticalSpeed() const { return LastLandingVerticalSpeed; }

	UFUNCTION(BlueprintPure, Category = "Character")
	EGait GetGait() const { return Gait; }

public:
	// Component getters
	UFUNCTION(BlueprintPure, Category = "Traversal")
	UMotionWarpingComponent* GetMotionWarpingComponent() const { return MotionWarpingComponent; }

	UFUNCTION(BlueprintPure, Category = "Traversal")
	UParkourTraversalComponent* GetParkourTraversalComponent() const { return ParkourTraversalComponent; }
};
