// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TantrumGameWidget.h"

#include "TantrumGameModeBase.generated.h"

/**
 * This class only lives on the server and determines what kind of game we're playing (deathmatch, capture the flag, ...)
 */
UCLASS()
class TANTRUM_API ATantrumGameModeBase : public AGameModeBase {
    GENERATED_BODY()

public:
    ATantrumGameModeBase();

    void RestartGame();

protected:
    void BeginPlay() override;
    void RestartPlayer(AController* newPlayer) override;

private:
    void _attemptStartGame();
    void _displayCountdown();
    void _startGame();

    UFUNCTION(BlueprintCallable, Category = "Game Details")
    void _setNumExpectedPlayers(const uint8 numExpectedPlayers) { _numExpectedPlayers = numExpectedPlayers; }

    UPROPERTY(EditAnywhere, Category = "Game Details")
    uint8 _numExpectedPlayers = 1u;

    UPROPERTY(EditAnywhere, Category = "Game Details")
    float _gameCountdownDuration = 2.0f;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UTantrumGameWidget> _gameWidgetClass = nullptr;
};
