// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Objects/Triggers/ConcreteAscentTriggerActorBase.h"
#include "GoalTriggerActor.generated.h"

/**
 * 
 */
UCLASS()
class CONCRETEASCENT_API AGoalTriggerActor : public AConcreteAscentTriggerActorBase
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Goal")
	bool bIsGoalActivated = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Goal")
	bool bIsCleared = false;

public:
	virtual void OnPlayerEntered(AConcreteAscentCharacter* Character) override;

	UFUNCTION(BlueprintCallable, Category = "Goal")
	bool CheckClearCondition(AConcreteAscentCharacter* Character) const;

	UFUNCTION(BlueprintPure, Category = "Goal")
	bool IsGoalActivated() const { return bIsGoalActivated; }

	UFUNCTION(BlueprintPure, Category = "Goal")
	bool IsCleared() const { return bIsCleared; }
};

