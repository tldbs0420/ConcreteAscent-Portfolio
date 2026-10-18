// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/ParkourMotionData.h"

UAnimMontage* UParkourMotionData::GetLedgeMoveMontage(float Direction) const
{
	return Direction < 0.f ? LedgeMoveLeftMontage : LedgeMoveRightMontage;
}
