// Copyright Epic Games, Inc. All Rights Reserved.


#include "Kismet/GameplayStatics.h"
#include "TantrumGameModeBase.h"

EGameState ATantrumGameModeBase::GetCurrentGameState() const {
    return _gameState;
}

void ATantrumGameModeBase::PlayerReachedEnd(APlayerController* controller) {
    _gameState = EGameState::GameOver;

    const auto gameWidget = _gameWidgets.Find(controller);
    check(gameWidget != nullptr);

    (*gameWidget)->LevelComplete();
    FInputModeUIOnly inputMode;

    const auto pc = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    pc->SetInputMode(inputMode);
    pc->SetShowMouseCursor(true);
}

void ATantrumGameModeBase::ReceivePlayer(APlayerController* controller) {
    _attemptStartGame();
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

    for (auto iterator = GetWorld()->GetPlayerControllerIterator(); iterator; ++iterator) {
        auto playerController = iterator->Get();
        if (IsValid(playerController) && playerController->PlayerState && !MustSpectate(playerController)) {
            if (auto gameWidget = CreateWidget<UTantrumGameWidget>(playerController, _gameWidgetClass)) {
                gameWidget->AddToPlayerScreen();
                gameWidget->StartCountdown(_gameCountdownDuration, this);
                _gameWidgets.Emplace(playerController, gameWidget);
            }
        }
    }
}

void ATantrumGameModeBase::_attemptStartGame() {
    if (GetNumPlayers() == _numExpectedPlayers) {
        _displayCountdown();
        FTimerHandle timerHandle;
        GetWorld()->GetTimerManager().SetTimer(timerHandle, this, &ATantrumGameModeBase::_startGame, _gameCountdownDuration, false);
    }
}

void ATantrumGameModeBase::_startGame() {
    // This is needed, otherwise if you restart the level via "retry" button you'll still be in UIOnly mode.
    FInputModeGameOnly inputMode;
    
    const auto pc = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    pc->SetInputMode(inputMode);
    pc->SetShowMouseCursor(false);

    _gameState = EGameState::Playing;
}
