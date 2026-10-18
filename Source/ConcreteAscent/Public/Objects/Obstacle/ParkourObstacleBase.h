// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ParkourObstacleBase.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class UBoxComponent;

UCLASS()
class CONCRETEASCENT_API AParkourObstacleBase : public AActor
{
	GENERATED_BODY()
	
public:
	AParkourObstacleBase();

protected:
	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> ObstacleMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> TraversalBounds;

	// Cached traversal values
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traversal")
	float Height = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traversal")
	float Thickness = 30.f;

public:
	virtual void OnConstruction(const FTransform& Transform) override;

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Traversal")
	bool GetTraversalLedgeData(
		const FHitResult& HitResult,
		const FVector& ActorLocation,
		FVector& OutFrontLedgeLocation,
		FVector& OutFrontLedgeNormal,
		FVector& OutBackLedgeLocation,
		FVector& OutBackLedgeNormal,
		float& OutObstacleHeight,
		float& OutObstacleDepth
	) const;

	virtual bool GetTraversalLedgeData_Implementation(
		const FHitResult& HitResult,
		const FVector& ActorLocation,
		FVector& OutFrontLedgeLocation,
		FVector& OutFrontLedgeNormal,
		FVector& OutBackLedgeLocation,
		FVector& OutBackLedgeNormal,
		float& OutObstacleHeight,
		float& OutObstacleDepth
	) const;

public:
	UFUNCTION(BlueprintPure, Category = "Traversal")
	float GetHeight() const { return Height; }

	UFUNCTION(BlueprintPure, Category = "Traversal")
	float GetThickness() const { return Thickness; }
};
