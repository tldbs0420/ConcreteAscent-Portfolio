// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/ConcreteAscentTypes.h"
#include "ParkourTraversalComponent.generated.h"

class AConcreteAscentCharacter;
class AParkourObstacleBase;
class ALedgeObstacle;
class UParkourMotionData;
class UAnimMontage;

UCLASS(Blueprintable, BlueprintType, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CONCRETEASCENT_API UParkourTraversalComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UParkourTraversalComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	// Owner
	UPROPERTY(Transient)
	TObjectPtr<AConcreteAscentCharacter> OwnerCharacter;

	// Data
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Traversal")
	TObjectPtr<UParkourMotionData> MotionData;

	// Runtime obstacle state
	UPROPERTY(Transient)
	TObjectPtr<ALedgeObstacle> CurrentLedge;

	UPROPERTY(Transient)
	FHitResult LastObstacleHit;

	// Runtime traversal state
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal")
	float CurrentLedgeOffset = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal|Ledge")
	FVector CurrentLedgeCenter = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal|Ledge")
	FVector CurrentLedgeRight = FVector::RightVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal|Ledge")
	FVector CurrentLedgeNormal = FVector::ForwardVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal|Ledge")
	float CurrentLedgeMinOffset = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal|Ledge")
	float CurrentLedgeMaxOffset = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal|Ledge")
	bool bIsLedgeMoving = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Traversal|Ledge")
	float LedgeMoveStepDistance = 80.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Traversal|Ledge")
	float LedgeEdgePadding = 15.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Traversal|Ledge")
	float LedgeMoveDefaultDuration = 0.35f;

	float LedgeMoveStartOffset = 0.f;
	float LedgeMoveTargetOffset = 0.f;
	float LedgeMoveElapsedTime = 0.f;
	float LedgeMoveDuration = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal|Ledge")
	bool bIsLedgeClimbing = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal|Ledge")
	bool bIsCurrentLedgeCollisionIgnored = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Traversal|Ledge")
	float LedgeClimbDefaultDuration = 0.7f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Traversal|Ledge")
	float LedgeClimbStandForwardOffset = 20.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Traversal|Ledge")
	float LedgeClimbStandZOffset = 3.f;

	FVector PendingClimbStandLocation = FVector::ZeroVector;
	FRotator PendingClimbStandRotation = FRotator::ZeroRotator;

	FTimerHandle LedgeClimbFinishTimerHandle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal")
	FTraversalCheckResult CurrentTraversalResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal")
	FTraversalChooserInputs CurrentTraversalInputs;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal")
	FTraversalChooserOutputs CurrentTraversalOutputs;

	// Trace settings
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Traversal|Trace")
	TEnumAsByte<ECollisionChannel> TraversableTraceChannel = ECC_GameTraceChannel1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Traversal|Trace")
	TEnumAsByte<ECollisionChannel> RoomTraceChannel = ECC_Visibility;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Traversal|Trace")
	float TraceForwardDistance = 150.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Traversal|Trace")
	float TraceOriginZOffset = 50.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Traversal|Trace")
	float LedgeRoomPadding = 2.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Traversal|Trace")
	float FloorCheckExtraDistance = 100.f;

protected:
	// Trace helpers
	bool CapsuleSweep(const FVector& Start, const FVector& End, ECollisionChannel Channel, FHitResult& OutHit) const;
	bool HasCapsuleRoom(const FVector& Start, const FVector& End, ECollisionChannel Channel, FHitResult& OutHit) const;

	// Traversal data helpers
	FTraversalCheckResult BuildTraversalCheckResult(AParkourObstacleBase* ObstacleBase);
	FTraversalChooserInputs MakeChooserInputsFromCheckResult(const FTraversalCheckResult& CheckResult) const;

public:
	// Traversal flow
	UFUNCTION(BlueprintCallable, Category = "Traversal")
	AParkourObstacleBase* DetectObstacle();

	UFUNCTION(BlueprintCallable, Category = "Traversal")
	FTraversalChooserInputs EvaluateTraversal(AParkourObstacleBase* ObstacleBase);

	UFUNCTION(BlueprintCallable, Category = "Traversal")
	bool StartTraversal(FTraversalCheckResult& OutResult);

	// Ledge flow
	UFUNCTION(BlueprintCallable, Category = "Traversal|Ledge")
	bool TryAirLedgeGrab(FTraversalCheckResult& OutResult);

	UFUNCTION(BlueprintCallable, Category = "Traversal")
	void MoveAlongLedge(float Direction);

	UFUNCTION(BlueprintCallable, Category = "Traversal")
	void ClimbFromLedge();

	UFUNCTION(BlueprintCallable, Category = "Traversal")
	void DropFromLedge();

	UFUNCTION(BlueprintCallable, Category = "Traversal")
	float PlayLedgeMontage(ETraversalAction Action);
	void FinishLedgeMove();
	bool CanClimbFromLedge(FVector& OutStandLocation, FRotator& OutStandRotation) const;
	void FinishLedgeClimb();
	void OnLedgeClimbMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void ResetLedgeRuntimeState();
	void StopLedgeMontages(float BlendOutTime = 0.05f);
	void SetCurrentLedgeCollisionIgnored(bool bIgnore);

	// Montage selection
	UFUNCTION(BlueprintCallable, Category = "Traversal")
	TArray<UAnimMontage*> BuildValidTraversalMontages(const FTraversalChooserInputs& Inputs,FTraversalChooserOutputs& OutOutputs);

	bool FindPoseSearchBranchInEndTime(const UAnimMontage* Montage, float& OutEndTime) const;

public:
	// Blueprint extension points
	UFUNCTION(BlueprintImplementableEvent, Category = "Traversal")
	void BP_UpdatePoseSearchPlayerActor(AActor* PlayerActor);

	UFUNCTION(BlueprintImplementableEvent, Category = "Traversal")
	void BP_EvaluateTraversalChooser(const FTraversalChooserInputs& Inputs, FTraversalChooserOutputs& Outputs, TArray<UAnimMontage*>& OutMontages);
};
