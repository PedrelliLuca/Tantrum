// Fill out your copyright notice in the Description page of Project Settings.

#include "TantrumLevelEndTrigger.h"

#include "TantrumCharacterBase.h"
#include "TantrumGameStateBase.h"

ATantrumLevelEndTrigger::ATantrumLevelEndTrigger() {
    OnActorBeginOverlap.AddDynamic(this, &ATantrumLevelEndTrigger::_communicateGameEnd);
}

void ATantrumLevelEndTrigger::_communicateGameEnd(AActor* overlappedActor, AActor* otherActor) {
    /* ATantrumLevelEndTrigger will be on every Client's machine => this function will be called every time a ATantrumCharacterBase makes it to this trigger,
     * even if it's a replica. However, even if a replica caused this trigger, it's the authoritative machine controls movement and should, therefore, validates
     * the actors' position in the world. This is why we ask to be the authority here. This way we prevent cheating: if one of the client somehow manages to
     * teleport to the end trigger but the server doesn't have the client's character there, then they won't get the victory.
     */
    if (!HasAuthority()) {
        return;
    }

    // Every game instance (there is one per client and one on the server) will have a game state
    if (const auto tantrumGS = GetWorld()->GetGameState<ATantrumGameStateBase>()) {
        tantrumGS->OnPlayerReachedEnd(Cast<ATantrumCharacterBase>(otherActor));
    }
}
