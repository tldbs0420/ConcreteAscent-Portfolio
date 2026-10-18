// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/ConcreteAscentGameModeBase.h"
#include "Objects/Triggers/CheckpointActor.h"
#include "Player/ConcreteAscentCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Player/Controller/ConcreteAscentPlayerController.h"


AConcreteAscentGameModeBase::AConcreteAscentGameModeBase()
{
}

void AConcreteAscentGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	PlayerCharacter = Cast<AConcreteAscentCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));

	if (PlayerCharacter)
		InitialRespawnTransform = PlayerCharacter->GetActorTransform();
}

void AConcreteAscentGameModeBase::StartRespawnFadeIn()
{
	AConcreteAscentPlayerController* PlayerController = Cast<AConcreteAscentPlayerController>(UGameplayStatics::GetPlayerController(this, 0));

	if (!PlayerController)
		return;

	const float FadeInDuration = PlayerController->StartRespawnFadeIn();

	GetWorldTimerManager().ClearTimer(RespawnFinishTimerHandle);
	GetWorldTimerManager().SetTimer(
		RespawnFinishTimerHandle,
		this,
		&AConcreteAscentGameModeBase::FinishRespawnSequence,
		FadeInDuration,
		false
	);
}

void AConcreteAscentGameModeBase::FinishRespawnAfterFade()
{
	if (PlayerCharacter)
		PlayerCharacter->RespawnAt(PendingRespawnTransform);

	GetWorldTimerManager().ClearTimer(RespawnFadeInTimerHandle);
	GetWorldTimerManager().SetTimer(
		RespawnFadeInTimerHandle,
		this,
		&AConcreteAscentGameModeBase::StartRespawnFadeIn,
		RespawnBlackHoldDuration,
		false
	);
}

void AConcreteAscentGameModeBase::FinishRespawnSequence()
{
	AConcreteAscentPlayerController* PlayerController = Cast<AConcreteAscentPlayerController>(UGameplayStatics::GetPlayerController(this, 0));

	if (PlayerController)
		PlayerController->FinishRespawnFadeIn();
}

void AConcreteAscentGameModeBase::SetCurrentCheckpoint(ACheckpointActor* NewCheckpoint)
{
	if (CurrentCheckpoint) CurrentCheckpoint->SetActivated(true);
	CurrentCheckpoint = NewCheckpoint;
	OnCheckpointActivated.Broadcast();
}

void AConcreteAscentGameModeBase::SetClearTime(float InClearTime)
{
	ClearTime = InClearTime;
}

void AConcreteAscentGameModeBase::HandleRespawnRequest()
{
	if (!PlayerCharacter || !CanRespawn())
		return;

	PendingRespawnTransform = GetRespawnTransform();

	AConcreteAscentPlayerController* PlayerController = Cast<AConcreteAscentPlayerController>(UGameplayStatics::GetPlayerController(this, 0));
	if (!PlayerController)
	{
		PlayerCharacter->RespawnAt(PendingRespawnTransform);
		return;
	}

	const float FadeOutDuration = PlayerController->StartRespawnFadeOut();
	if (FadeOutDuration <= 0.f)
	{
		FinishRespawnAfterFade();
		return;
	}

	GetWorldTimerManager().ClearTimer(RespawnFadeTimerHandle);
	GetWorldTimerManager().SetTimer(
		RespawnFadeTimerHandle,
		this,
		&AConcreteAscentGameModeBase::FinishRespawnAfterFade,
		FadeOutDuration,
		false
	);
}

void AConcreteAscentGameModeBase::HandleGoalReached()
{
	if (CanClear())
		HandleGameClear();
}

void AConcreteAscentGameModeBase::HandleGameClear()
{
	if (bGameCleared)
		return;

	bGameCleared = true;
	AConcreteAscentPlayerController* PlayerController = Cast<AConcreteAscentPlayerController>(UGameplayStatics::GetPlayerController(this, 0));

	if (!PlayerController)
		return;

	PlayerController->ShowGameClearUI();
}

void AConcreteAscentGameModeBase::HandleGameFail()
{
	if (bGameCleared)
		return;

	AConcreteAscentPlayerController* PlayerController = Cast<AConcreteAscentPlayerController>(UGameplayStatics::GetPlayerController(this, 0));

	if (!PlayerController)
		return;

	PlayerController->ShowGameFailUI();
}

FTransform AConcreteAscentGameModeBase::GetRespawnTransform()
{
	if (CurrentCheckpoint)
		return CurrentCheckpoint->GetRespawnTransform();

	return InitialRespawnTransform;
}