// Copyright Epic Games, Inc. All Rights Reserved.


#include "TantrumGameModeBase.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TantrumGameStateBase.h"
#include "TantrumPlayerController.h"
#include "TantrumPlayerState.h"

ATantrumGameModeBase::ATantrumGameModeBase() {
    PrimaryActorTick.bCanEverTick = false;
}

void ATantrumGameModeBase::RestartGame() {
    ResetLevel();

	for (FConstPlayerControllerIterator iterator = GetWorld()->GetPlayerControllerIterator(); iterator; ++iterator) {
		const auto playerController = iterator->Get();

		if (playerController && playerController->PlayerState && !MustSpectate(playerController)) {
			//call something to clean up the hud 
			if (const auto tantrumnPlayerController = Cast<ATantrumPlayerController>(playerController)) {
				tantrumnPlayerController->ClientRestartGame();
			}
			RestartPlayer(playerController);
		}
	}
}

void ATantrumGameModeBase::RestartPlayer(AController* newPlayer) {
    Super::RestartPlayer(newPlayer);

	if (const auto playerController = Cast<APlayerController>(newPlayer)) {
		if (playerController->GetCharacter() && playerController->GetCharacter()->GetCharacterMovement()) {
			playerController->GetCharacter()->GetCharacterMovement()->SetMovementMode(MOVE_Walking);

			const auto playerState = playerController->GetPlayerState<ATantrumPlayerState>();
			if (playerState) {
				playerState->SetCurrentState(EPlayerGameState::Waiting);
			}
		}
	}

	_attemptStartGame();
}

void ATantrumGameModeBase::BeginPlay() {
    Super::BeginPlay();

    if (const auto tantrumGameState = GetGameState<ATantrumGameStateBase>()) {
        tantrumGameState->SetGameState(EGameState::Waiting);
    }
}


void ATantrumGameModeBase::_attemptStartGame() {
    if (const auto tantrumGameState = GetGameState<ATantrumGameStateBase>()) {
        tantrumGameState->SetGameState(EGameState::Waiting);
    }

    if (GetNumPlayers() == _numExpectedPlayers) {
        // This needs to be replicated, call a function on game instance and replicate
        _displayCountdown();
        if (_gameCountdownDuration > SMALL_NUMBER) {
            FTimerHandle timerHandle;
            GetWorld()->GetTimerManager().SetTimer(timerHandle, this, &ATantrumGameModeBase::_startGame, _gameCountdownDuration, false);
        } else {
            _startGame();
        }

    } else {
	    UE_LOG(LogTemp, Warning, TEXT("%s(): Number of Players different from the expected one"), *FString{__FUNCTION__});
    }
}

void ATantrumGameModeBase::_displayCountdown() {
    for (auto iterator = GetWorld()->GetPlayerControllerIterator(); iterator; ++iterator) {
        auto playerController = iterator->Get();
        if (IsValid(playerController) && playerController->PlayerState && !MustSpectate(playerController)) {
            if (const auto tantrumPlayerController = Cast<ATantrumPlayerController>(playerController)) {
                tantrumPlayerController->ClientDisplayCountdown(_gameCountdownDuration);
            }
        }
    }
}

void ATantrumGameModeBase::_startGame() {
    if (const auto tantrumGameState = GetGameState<ATantrumGameStateBase>())
	{
		tantrumGameState->SetGameState(EGameState::Playing);
		tantrumGameState->ClearResults();
	}

	for (FConstPlayerControllerIterator iterator = GetWorld()->GetPlayerControllerIterator(); iterator; ++iterator)
	{
		const auto playerController = iterator->Get();
		if (playerController && playerController->PlayerState && !MustSpectate(playerController))
		{
			FInputModeGameOnly InputMode;
			playerController->SetInputMode(InputMode);
			playerController->SetShowMouseCursor(false);

			const auto playerState = playerController->GetPlayerState<ATantrumPlayerState>();
			if (playerState)
			{
				playerState->SetCurrentState(EPlayerGameState::Playing);
				playerState->SetIsWinner(false);
			}
		}
	}
}
