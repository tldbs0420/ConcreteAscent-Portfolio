// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/ConcreteAscentStandardGameMode.h"
#include "Objects/Triggers/CheckpointActor.h"

bool AConcreteAscentStandardGameMode::CanClear()
{
	return true;
}

bool AConcreteAscentStandardGameMode::CanRespawn()
{
	return PlayerCharacter != nullptr;
}