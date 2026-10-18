// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Objects/Triggers/ConcreteAscentTriggerActorBase.h"
#include "CheckpointActor.generated.h"

/**
 * 
 */

class USceneComponent;
class AConcreteAscentCharacter;

UCLASS()
class CONCRETEASCENT_API ACheckpointActor : public AConcreteAscentTriggerActorBase
{
	GENERATED_BODY()
	
public:
	ACheckpointActor();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Checkpoint")
	TObjectPtr<USceneComponent> RespawnPoint;

public:
	virtual void OnPlayerEntered(AConcreteAscentCharacter* Character) override;

	UFUNCTION(BlueprintPure, Category = "Checkpoint")
	USceneComponent* GetRespawnPoint() const { return RespawnPoint; }

	UFUNCTION(BlueprintPure, Category = "Checkpoint")
	FTransform GetRespawnTransform() const;

	UFUNCTION(BlueprintCallable, Category = "Checkpoint")
	void SetActivated(bool bInActivated);

	UFUNCTION(BlueprintImplementableEvent, Category = "Checkpoint")
	void BP_OnCheckpointActivated(bool bIsActivated);
};
