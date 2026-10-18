// Fill out your copyright notice in the Description page of Project Settings.


#include "Objects/Triggers/ConcreteAscentTriggerActorBase.h"
#include "Components/BoxComponent.h"
#include "Player/ConcreteAscentCharacter.h"
#include "GameMode/ConcreteAscentGameModeBase.h"
#include "Engine/World.h"

AConcreteAscentTriggerActorBase::AConcreteAscentTriggerActorBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
	SetRootComponent(TriggerVolume);

	TriggerVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void AConcreteAscentTriggerActorBase::BeginPlay()
{
	Super::BeginPlay();

	CachedGameMode = GetWorld() ? Cast<AConcreteAscentGameModeBase>(GetWorld()->GetAuthGameMode()) : nullptr;
	if (TriggerVolume)
		TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &AConcreteAscentTriggerActorBase::HandleTriggerBeginOverlap);
}

void AConcreteAscentTriggerActorBase::HandleTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AConcreteAscentCharacter* Character = Cast<AConcreteAscentCharacter>(OtherActor))
	{
		if (CanTrigger(Character))
			OnPlayerEntered(Character);
	}
}

bool AConcreteAscentTriggerActorBase::CanTrigger(AConcreteAscentCharacter* Character) const
{
	return bIsEnabled && Character != nullptr;
}