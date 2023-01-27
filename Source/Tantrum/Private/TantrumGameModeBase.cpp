// Copyright Epic Games, Inc. All Rights Reserved.


#include "Kismet/GameplayStatics.h"
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
    if (!IsValid(_gameWidgetClass)) {
        UE_LOG(LogTemp, Error, TEXT("%s: missing game widget class!!"), *FString{__FUNCTION__});
        return;
    }

    _gameWidget = CreateWidget<UTantrumGameWidget>(UGameplayStatics::GetPlayerController(GetWorld(), 0), _gameWidgetClass, TEXT("Game Widget"));
    _gameWidget->AddToViewport();
    _gameWidget->StartCountdown(_gameCountdownDuration, this);
}

void ATantrumGameModeBase::_startGame() {
    _gameState = EGameState::Playing;
}
