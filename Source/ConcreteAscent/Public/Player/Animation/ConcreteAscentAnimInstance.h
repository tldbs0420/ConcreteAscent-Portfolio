// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Types/ConcreteAscentTypes.h"
#include "ConcreteAscentAnimInstance.generated.h"

class AConcreteAscentCharacter;
class UParkourMotionData;

/**
 * 
 */
UCLASS()
class CONCRETEASCENT_API UConcreteAscentAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	UPROPERTY(Transient, BlueprintReadWrite, Category = "Animation")
	TObjectPtr<AConcreteAscentCharacter> OwnerCharacter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TObjectPtr<UParkourMotionData> MotionData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	float Speed = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	float Direction = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	float HeavyLandThreshold = 700.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	bool bIsInAir = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	EMovementState MovementState = EMovementState::OnGround;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	EMovementState MovementStateLastFrame = EMovementState::OnGround;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	EMoveMode MoveMode = EMoveMode::Idle;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	EMoveMode MoveModeLastFrame = EMoveMode::Idle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	EGait Gait = EGait::Walk;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	EGait GaitLastFrame = EGait::Walk;

public:
	UFUNCTION(BlueprintCallable, Category = "Animation")
	void GetGaitFromPlayer();
};
