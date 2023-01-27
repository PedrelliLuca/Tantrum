// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerVolume.h"
#include "TantrumGameModeBase.h"

#include "TantrumLevelEndTrigger.generated.h"

/**
 * 
 */
UCLASS()
class TANTRUM_API ATantrumLevelEndTrigger : public ATriggerVolume {
	GENERATED_BODY()
	
public:
	ATantrumLevelEndTrigger();

protected:
	void BeginPlay() override;

private:
	TWeakObjectPtr<ATantrumGameModeBase> _gameMode = nullptr;

	UFUNCTION()
	void _communicateGameEnd(AActor* overlappedActor, AActor* otherActor);
};
