// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Types/ConcreteAscentTypes.h"
#include "ParkourMotionData.generated.h"

class UAnimMontage;

/**
 * 
 */
UCLASS()
class CONCRETEASCENT_API UParkourMotionData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ledge")
	TObjectPtr<UAnimMontage> LedgeGrabMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ledge")
	TObjectPtr<UAnimMontage> LedgeMoveLeftMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ledge")
	TObjectPtr<UAnimMontage> LedgeMoveRightMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ledge")
	TObjectPtr<UAnimMontage> LedgeClimbUpMontage;

	UFUNCTION(BlueprintPure, Category = "Traversal")
	UAnimMontage* GetLedgeGrabMontage() const { return LedgeGrabMontage; }

	UFUNCTION(BlueprintPure, Category = "Traversal")
	UAnimMontage* GetLedgeMoveMontage(float Direction) const;

	UFUNCTION(BlueprintPure, Category = "Traversal")
	UAnimMontage* GetLedgeClimbUpMontage() const { return LedgeClimbUpMontage; }
};
