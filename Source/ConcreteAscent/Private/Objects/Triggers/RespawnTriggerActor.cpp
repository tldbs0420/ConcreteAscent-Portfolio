// Fill out your copyright notice in the Description page of Project Settings.


#include "Objects/Triggers/RespawnTriggerActor.h"
#include "GameMode/ConcreteAscentGameModeBase.h"

void ARespawnTriggerActor::OnPlayerEntered(AConcreteAscentCharacter* Character)
{
	if (CachedGameMode)
		CachedGameMode->HandleRespawnRequest();
}
