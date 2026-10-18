// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ConcreteAscentTriggerActorBase.generated.h"

class AConcreteAscentCharacter;
class AConcreteAscentGameModeBase;
class UBoxComponent;

// Unreal식 추상 클래스 : Abstract로는 실제 Blueprint를 만들 수 없다.
UCLASS(Abstract)
class CONCRETEASCENT_API AConcreteAscentTriggerActorBase : public AActor
{
	GENERATED_BODY()

public:
	AConcreteAscentTriggerActorBase();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trigger")
	TObjectPtr<UBoxComponent> TriggerVolume;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
	bool bIsEnabled = true;

	UPROPERTY(Transient)
	TObjectPtr<AConcreteAscentGameModeBase> CachedGameMode;

	UFUNCTION()
	void HandleTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:
	UFUNCTION(BlueprintCallable, Category = "Trigger")
	virtual bool CanTrigger(AConcreteAscentCharacter* Character) const;

	// Unreal식 추상 함수 : 자식 Trigger들은 이걸 구현해야한다.
	UFUNCTION(BlueprintCallable, Category = "Trigger")
	virtual void OnPlayerEntered(AConcreteAscentCharacter* Character) PURE_VIRTUAL(AConcreteAscentTriggerActorBase::OnPlayerEntered, );

	UFUNCTION(BlueprintPure, Category = "Trigger")
	bool IsEnabled() const { return bIsEnabled; }
};
