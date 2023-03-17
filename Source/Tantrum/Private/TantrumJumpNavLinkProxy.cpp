// Fill out your copyright notice in the Description page of Project Settings.

#include "TantrumJumpNavLinkProxy.h"

#include "TantrumCharacterBase.h"

void ATantrumJumpNavLinkProxy::BeginPlay() {
    Super::BeginPlay();

    OnSmartLinkReached.AddDynamic(this, &ATantrumJumpNavLinkProxy::_onSmartLinkReached);
}

void ATantrumJumpNavLinkProxy::_onSmartLinkReached(AActor* movingActor, const FVector& destinationPoint) {
    if (const auto tantrumCharacter = Cast<ATantrumCharacterBase>(movingActor); IsValid(tantrumCharacter)) {
        tantrumCharacter->Jump();
    }
}
