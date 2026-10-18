// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameMode/ConcreteAscentGameModeBase.h"
#include "ConcreteAscentStandardGameMode.generated.h"

/**
 * 
 */
UCLASS()
class CONCRETEASCENT_API AConcreteAscentStandardGameMode : public AConcreteAscentGameModeBase
{
	GENERATED_BODY()
	
public:
	virtual bool CanClear() override;
	virtual bool CanRespawn() override;
};
