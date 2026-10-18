// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Objects/Triggers/ConcreteAscentTriggerActorBase.h"
#include "RespawnTriggerActor.generated.h"

/**
 * 
 */
UCLASS()
class CONCRETEASCENT_API ARespawnTriggerActor : public AConcreteAscentTriggerActorBase
{
	GENERATED_BODY()
	
public:
	virtual void OnPlayerEntered(AConcreteAscentCharacter* Character) override;
};
