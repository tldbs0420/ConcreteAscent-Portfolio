// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Objects/Obstacle/ParkourObstacleBase.h"
#include "LedgeObstacle.generated.h"

/**
 * 
 */
UCLASS()
class CONCRETEASCENT_API ALedgeObstacle : public AParkourObstacleBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Traversal|Ledge")
	bool GetLedgeMoveSegment(const FVector& FrontNormal, FVector& OutCenter, FVector& OutRight, float& OutHalfLength) const;
};
