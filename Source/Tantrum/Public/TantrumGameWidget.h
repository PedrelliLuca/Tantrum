// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "TantrumGameWidget.generated.h"

/**
 * 
 */
UCLASS()
class TANTRUM_API UTantrumGameWidget : public UUserWidget {
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent)
	void StartCountdown(float countdownTime, class ATantrumGameModeBase* gameMode);

	UFUNCTION(BlueprintImplementableEvent)
	void LevelComplete();
};
