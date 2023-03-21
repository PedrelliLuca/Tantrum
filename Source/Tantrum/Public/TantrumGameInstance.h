// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Engine/GameInstance.h"
#include "TantrumGameStateBase.h"
#include "TantrumGameWidget.h"
#include "TantrumPlayerController.h"

#include "TantrumGameInstance.generated.h"

/**
 * Exists throughout the entire game, whether you're switching level or you're in a menu screen or whatever... represents the fact that the game is running.
 */
UCLASS()
class TANTRUM_API UTantrumGameInstance : public UGameInstance {
    GENERATED_BODY()

    // NOTE: the content of this class was stripped away when we decided that it was better to have the widget in the Game Mode, since that has
    // to change depending on it. I am leaving it as a comment for posterity.
    //
    // public:
    //    void DisplayCountdown(float gameCountdownDuration, ATantrumPlayerController* tantrumPlayerController);
    //    void DisplayLevelComplete(ATantrumPlayerController* tantrumPlayerController);
    //
    //    UFUNCTION(BlueprintPure)
    //    ATantrumGameStateBase* GetGameState() const { return _tantrumGameStateBase; }
    //
    //    UFUNCTION(BlueprintCallable)
    //    void OnRetrySelected(ATantrumPlayerController* tantrumPlayerController);
    //
    //    UFUNCTION()
    //    void RestartGame(ATantrumPlayerController* tantrumPlayerController);
    //
    // private:
    //    UFUNCTION()
    //    void _onGameStateSet(AGameStateBase* gameStateBase);
    //
    //    UPROPERTY(EditAnywhere, Category = "Widget")
    //    TSubclassOf<UTantrumGameWidget> _gameWidgetClass = nullptr;
    //
    //    UPROPERTY()
    //    TMap<APlayerController*, UTantrumGameWidget*> _gameWidgets;
    //
    //    UPROPERTY()
    //    TObjectPtr<ATantrumGameStateBase> _tantrumGameStateBase;
};
