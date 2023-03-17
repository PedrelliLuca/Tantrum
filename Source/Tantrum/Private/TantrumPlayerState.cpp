// Fill out your copyright notice in the Description page of Project Settings.

#include "TantrumPlayerState.h"

#include "Net/UnrealNetwork.h"

void ATantrumPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    FDoRepLifetimeParams sharedParams;
    sharedParams.bIsPushBased = true;

    DOREPLIFETIME_WITH_PARAMS_FAST(ATantrumPlayerState, _currentState, sharedParams);
    DOREPLIFETIME_WITH_PARAMS_FAST(ATantrumPlayerState, _isWinner, sharedParams);
}
