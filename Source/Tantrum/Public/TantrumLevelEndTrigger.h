// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerVolume.h"

#include "TantrumLevelEndTrigger.generated.h"

/**
 * 
 */
UCLASS()
class TANTRUM_API ATantrumLevelEndTrigger : public ATriggerVolume {
	GENERATED_BODY()
	
public:
	ATantrumLevelEndTrigger();

private:
	UFUNCTION()
	void _communicateGameEnd(AActor* overlappedActor, AActor* otherActor);
};
