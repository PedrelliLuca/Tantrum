// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Engine/GameInstance.h"
#include "TantrumGameStateBase.h"
#include "TantrumGameWidget.h"
#include "TantrumPlayerController.h"

#include "TantrumGameInstance.generated.h"

/**
 * Exists throughout the entire game, whether you're switching level or you're in a menu screen or whatever...
 */
UCLASS()
class TANTRUM_API UTantrumGameInstance : public UGameInstance {
	GENERATED_BODY()

public:
	void DisplayCountdown(float gameCountdownDuration, ATantrumPlayerController* tantrumPlayerController);
	void DisplayLevelComplete(ATantrumPlayerController* tantrumPlayerController);
	
	UFUNCTION(BlueprintPure)
	ATantrumGameStateBase* GetGameState() const { return _tantrumGameStateBase;  }

	UFUNCTION(BlueprintCallable)
	void OnRetrySelected(ATantrumPlayerController* tantrumPlayerController);

	UFUNCTION()
	void RestartGame(ATantrumPlayerController* tantrumPlayerController);

private:
	UFUNCTION()
	void _onGameStateSet(AGameStateBase* gameStateBase);

	UPROPERTY(EditAnywhere, Category = "Widget")
	TSubclassOf<UTantrumGameWidget> _gameWidgetClass = nullptr;

	UPROPERTY()
	TMap<APlayerController*, UTantrumGameWidget*> _gameWidgets;

	UPROPERTY()
	TObjectPtr<ATantrumGameStateBase> _tantrumGameStateBase;
};
