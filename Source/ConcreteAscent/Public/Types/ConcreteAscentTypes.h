// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ConcreteAscentTypes.generated.h"

class UPrimitiveComponent;
class UAnimMontage;

UENUM(BlueprintType)
enum class EMovementState : uint8
{
	OnGround UMETA(DisplayName = "OnGround"),
	InAir UMETA(DisplayName = "InAir"),
	Hanging UMETA(DisplayName = "Hanging"),
};

UENUM(BlueprintType)
enum class EMoveMode : uint8
{
	Idle UMETA(DisplayName = "Idle"),
	Move UMETA(DisplayName = "Move"),
};

UENUM(BlueprintType)
enum class EGait : uint8
{
	Walk UMETA(DisplayName = "Walk"),
	Run UMETA(DisplayName = "Run"),
	Sprint UMETA(DisplayName = "Sprint"),
};

UENUM(BlueprintType)
enum class ETraversalAction : uint8
{
	None UMETA(DisplayName = "None"),
	Vault UMETA(DisplayName = "Vault"),
	Hurdle UMETA(DisplayName = "Hurdle"),
	Mantle UMETA(DisplayName = "Mantle"),
	LedgeGrab UMETA(DisplayName = "LedgeGrab"),
	LedgeClimbUp UMETA(DisplayName = "LedgeClimbUp"),
	LedgeMoveLeft UMETA(DisplayName = "LedgeMoveLeft"),
	LedgeMoveRight UMETA(DisplayName = "LedgeMoveRight"),
};

USTRUCT(BlueprintType)
struct CONCRETEASCENT_API FTraversalChooserInputs
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	ETraversalAction ActionType = ETraversalAction::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	TEnumAsByte<EMovementMode> MovementMode = MOVE_Walking;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	EGait Gait = EGait::Run;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	float Speed = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	bool bHasFrontLedge = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	bool bHasBackLedge = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	bool bHasBackFloor = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	float ObstacleHeight = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	float ObstacleDepth = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	float BackLedgeHeight = 0.f;
};

USTRUCT(BlueprintType)
struct CONCRETEASCENT_API FTraversalChooserOutputs
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	ETraversalAction ActionType = ETraversalAction::None;
};

USTRUCT(BlueprintType)
struct CONCRETEASCENT_API FTraversalCheckResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	ETraversalAction ActionType = ETraversalAction::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	bool bHasFrontLedge = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	FVector FrontLedgeLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	FVector FrontLedgeNormal = FVector::ForwardVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	bool bHasBackLedge = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	FVector BackLedgeLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	FVector BackLedgeNormal = FVector::ForwardVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	bool bHasBackFloor = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	FVector BackFloorLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	float ObstacleHeight = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	float ObstacleDepth = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	float BackLedgeHeight = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	TObjectPtr<UPrimitiveComponent> HitComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	TObjectPtr<UAnimMontage> ChosenMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	float StartTime = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	float PlayRate = 1.f;
};
