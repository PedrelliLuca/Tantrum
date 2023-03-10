// Fill out your copyright notice in the Description page of Project Settings.


#include "TantrumGameStateBase.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "TantrumCharacterBase.h"
#include "TantrumPlayerController.h"
#include "TantrumPlayerState.h"

void ATantrumGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams sharedParams;
	sharedParams.bIsPushBased = true;
	// sharedParams.Condition = COND_SkipOwner; // The machine that sent the replication request sets the new value locally, so there is no point in having the server sending it back.

	DOREPLIFETIME_WITH_PARAMS_FAST(ATantrumGameStateBase, _gameState, sharedParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(ATantrumGameStateBase, _results, sharedParams);
}

// Only ever called by Authority, see ATantrumLevelEndTrigger class
void ATantrumGameStateBase::OnPlayerReachedEnd(ATantrumCharacterBase* tantrumCharacter) {
	check(HasAuthority());

	if (const auto tantrumPC = tantrumCharacter->GetController<ATantrumPlayerController>()) {

		/* The controller will tell the instance to update the HUD. The widget must be displayed only on the client whose this game state belongs to. 
		 * Not on every other client, not on the server. The server has no authority over things like the widgets that you show or the input. */
		tantrumPC->ClientReachedEnd();
		tantrumCharacter->GetCharacterMovement()->DisableMovement();

		const auto playerState = tantrumPC->GetPlayerState<ATantrumPlayerState>();
		if (IsValid(playerState)) {
			const bool isWinner = _results.Num() == 0;
			playerState->SetIsWinner(isWinner);
			playerState->SetCurrentState(EPlayerGameState::Finished);
		}

		FGameResult result;
		result.Name = tantrumCharacter->GetName();
		result.Time = 5.0f; // This is a fake result lol
		_results.Emplace(result);

		// This won't work  once JIP (Join In Progress) is enabled.
		if (_results.Num() == PlayerArray.Num()) {
			_gameState = EGameState::GameOver;
		}
	}
}

void ATantrumGameStateBase::OnRep_GameState(const EGameState& oldGameState) {
	UE_LOG(LogTemp, Warning, TEXT("%s(): oldGameState: %s"), *FString{ __FUNCTION__ }, *UEnum::GetDisplayValueAsText(oldGameState).ToString());
	UE_LOG(LogTemp, Warning, TEXT("%s(): _gameState: %s"), *FString{ __FUNCTION__ }, *UEnum::GetDisplayValueAsText(_gameState).ToString());

}
