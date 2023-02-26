// Fill out your copyright notice in the Description page of Project Settings.


#include "TantrumGameStateBase.h"
#include "TantrumPlayerController.h"

// Only ever called by Authority
void ATantrumGameStateBase::OnPlayerReachedEnd(ATantrumCharacterBase* tantrumCharacter) {
	check(HasAuthority());

	if (const auto tantrumPC = tantrumCharacter->GetController<ATantrumPlayerController>()) {

		/* Telling the client it reached the end. The widget must be displayed only on the client whose this game state belongs to. Not on every other
		 * client, not on the server. The server has no authority over things like the widgets that you show or the input. */
		tantrumPC->ClientReachedEnd();
	}
}
