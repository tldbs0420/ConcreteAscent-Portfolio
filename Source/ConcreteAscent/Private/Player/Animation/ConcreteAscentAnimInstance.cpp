// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/Animation/ConcreteAscentAnimInstance.h"
#include "Player/ConcreteAscentCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "KismetAnimationLibrary.h"
#include "Data/ParkourMotionData.h"
#include "Player/Components/ParkourTraversalComponent.h"

void UConcreteAscentAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	OwnerCharacter = Cast<AConcreteAscentCharacter>(TryGetPawnOwner());
}

void UConcreteAscentAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!OwnerCharacter)
		OwnerCharacter = Cast<AConcreteAscentCharacter>(TryGetPawnOwner());
}

void UConcreteAscentAnimInstance::GetGaitFromPlayer()
{
	if (!OwnerCharacter)
		return;
	Gait = OwnerCharacter->GetGait();
}