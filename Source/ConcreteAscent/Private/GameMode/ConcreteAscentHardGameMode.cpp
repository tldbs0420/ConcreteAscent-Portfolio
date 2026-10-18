// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/ConcreteAscentHardGameMode.h"

bool AConcreteAscentHardGameMode::CanClear()
{
	return true;
}

bool AConcreteAscentHardGameMode::CanRespawn()
{
	HandleGameFail();
	return false;
}
