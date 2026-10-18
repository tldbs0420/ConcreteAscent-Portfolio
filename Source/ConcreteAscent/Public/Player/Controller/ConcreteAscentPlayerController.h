// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ConcreteAscentPlayerController.generated.h"

class UEnhancedInputLocalPlayerSubsystem;
class UInputMappingContext;
class UUserWidget;

/**
 * 
 */
UCLASS()
class CONCRETEASCENT_API AConcreteAscentPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AConcreteAscentPlayerController();

protected:
	virtual void BeginPlay() override;

	FTimerHandle GameClearFadeTimerHandle;
	FTimerHandle GameFailFadeTimerHandle;

	void StartScreenFade(float FromAlpha, float ToAlpha, float Duration, bool bHoldWhenFinished);
	void FinishShowGameClearUI();
	void FinishShowGameFailUI();

	UPROPERTY(Transient)
	TObjectPtr<UEnhancedInputLocalPlayerSubsystem> InputSubsystem;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> GameplayMappingContext;

	UPROPERTY()
	TObjectPtr<UUserWidget> InGameHUDWidget;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> PauseMenuClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> PauseMenuWidget;

	UPROPERTY()
	TObjectPtr<UUserWidget> GameClearUIWidget;

	UPROPERTY()
	TObjectPtr<UUserWidget> GameFailUIWidget;

	UPROPERTY(EditDefaultsOnly, Category = "UI|Fade")
	float FadeToBlackDuration = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category = "UI|Fade")
	float FadeFromBlackDuration = 0.5f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	bool bPauseMenuOpen = false;

	bool bIsGameClearShowing = false;

	void CreateInGameHUD();

public:
	UFUNCTION(BlueprintCallable, Category = "Input")
	void CacheInputSubsystem();

	UFUNCTION(BlueprintCallable, Category = "Input")
	void SetupInputMapping();

	UFUNCTION(BlueprintCallable, Category = "Input")
	void AddMappingContext(UInputMappingContext* Context, int32 Priority = 0);

	UFUNCTION(BlueprintCallable, Category = "Input")
	void RemoveMappingContext(UInputMappingContext* Context);

	UFUNCTION(BlueprintCallable, Category = "Input")
	void SetGameplayInputMode();

	UFUNCTION(BlueprintCallable, Category = "Input")
	void SetGameAndUIInputMode(UUserWidget* FocusWidget);

	UFUNCTION(BlueprintCallable, Category = "Input")
	void SetUIOnlyInputMode(UUserWidget* FocusWidget);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void TogglePauseMenu();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void OpenPauseMenu();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ClosePauseMenu();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowGameClearUI();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowGameFailUI();

	UFUNCTION(BlueprintCallable, Category = "UI|Fade")
	void FadeToBlack(float Duration, bool bHoldWhenFinished = true);

	UFUNCTION(BlueprintPure, Category = "UI")
	UUserWidget* GetInGameHUDWidget() const { return InGameHUDWidget; }

	UFUNCTION(BlueprintCallable, Category = "UI|Fade")
	void FadeFromBlack(float Duration);

	UFUNCTION(BlueprintCallable, Category = "UI|Fade")
	float StartRespawnFadeOut();

	UFUNCTION(BlueprintCallable, Category = "UI|Fade")
	float StartRespawnFadeIn();

	UFUNCTION(BlueprintCallable, Category = "UI|Fade")
	void FinishRespawnFadeIn();
};
