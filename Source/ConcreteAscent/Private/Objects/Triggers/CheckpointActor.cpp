// Fill out your copyright notice in the Description page of Project Settings.


#include "Objects/Triggers/CheckpointActor.h"
#include "Components/SceneComponent.h"
#include "GameMode/ConcreteAscentGameModeBase.h"

ACheckpointActor::ACheckpointActor()
{
	RespawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("RespawnPoint"));
	RespawnPoint->SetupAttachment(RootComponent);
	RespawnPoint->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	RespawnPoint->SetRelativeRotation(FRotator::ZeroRotator);
}

FTransform ACheckpointActor::GetRespawnTransform() const
{
	return RespawnPoint ? RespawnPoint->GetComponentTransform() : GetActorTransform();
}

void ACheckpointActor::SetActivated(bool bInActivated)
{
	bIsEnabled = bInActivated;
	BP_OnCheckpointActivated(false);
}

void ACheckpointActor::OnPlayerEntered(AConcreteAscentCharacter* Character)
{
	if (!Character || !CachedGameMode)
		return;

	CachedGameMode->SetCurrentCheckpoint(this);

	// 한 번 활성화된 체크포인트는 중복 발동하지 않도록 비활성화한다.
	bIsEnabled = false;

	BP_OnCheckpointActivated(true);
}
