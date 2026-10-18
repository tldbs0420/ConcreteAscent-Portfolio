// Fill out your copyright notice in the Description page of Project Settings.


#include "Objects/Triggers/GoalTriggerActor.h"
#include "GameMode/ConcreteAscentGameModeBase.h"

void AGoalTriggerActor::OnPlayerEntered(AConcreteAscentCharacter* Character)
{
	if (!CheckClearCondition(Character))
	{
		return;
	}

	bIsCleared = true;
	if (CachedGameMode)
	{
		CachedGameMode->HandleGoalReached();
	}
}

bool AGoalTriggerActor::CheckClearCondition(AConcreteAscentCharacter* Character) const
{
	return bIsEnabled && bIsGoalActivated && Character != nullptr && !bIsCleared;
}