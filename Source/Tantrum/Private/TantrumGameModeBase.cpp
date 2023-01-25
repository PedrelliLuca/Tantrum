// Copyright Epic Games, Inc. All Rights Reserved.


#include "TantrumGameModeBase.h"

EGameState ATantrumGameModeBase::GetCurrentGameState() const {
    return _gameState;
}

void ATantrumGameModeBase::PlayerReachedEnd() {
    _gameState = EGameState::GameOver;

    // TODO: Update widget here
}

void ATantrumGameModeBase::BeginPlay() {
    Super::BeginPlay();

    _gameState = EGameState::Waiting;
    _displayCountdown();

    FTimerHandle handle{};
    GetWorld()->GetTimerManager().SetTimer(handle, this, &ATantrumGameModeBase::_startGame, _gameCountdownDuration, false);
}

void ATantrumGameModeBase::_displayCountdown() {
    // TODO: Create and store display widget.
}

void ATantrumGameModeBase::_startGame() {
    _gameState = EGameState::Playing;
}
