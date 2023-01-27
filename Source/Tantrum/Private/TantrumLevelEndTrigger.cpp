// Fill out your copyright notice in the Description page of Project Settings.


#include "TantrumLevelEndTrigger.h"

#include "TantrumCharacterBase.h"

ATantrumLevelEndTrigger::ATantrumLevelEndTrigger() {
	OnActorBeginOverlap.AddDynamic(this, &ATantrumLevelEndTrigger::_communicateGameEnd);
}

void ATantrumLevelEndTrigger::BeginPlay() {
	Super::BeginPlay();

	_gameMode = GetWorld()->GetAuthGameMode<ATantrumGameModeBase>();
}

void ATantrumLevelEndTrigger::_communicateGameEnd(AActor* overlappedActor, AActor* otherActor) {
	if (const auto tantrumCharacter = Cast<ATantrumCharacterBase>(otherActor)) {
		check(_gameMode.IsValid());
		_gameMode->PlayerReachedEnd();
	}
}
