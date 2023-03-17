// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "GameFramework/PlayerState.h"

#include "TantrumPlayerState.generated.h"

UENUM(BlueprintType)
enum class EPlayerGameState : uint8
{
    None UMETA(DisplayName = "None"),
    Waiting UMETA(DisplayName = "Waiting"),
    Playing UMETA(DisplayName = "Playing"),
    Finished UMETA(DisplayName = "Finished")
};

/**
 *
 */
UCLASS()
class TANTRUM_API ATantrumPlayerState : public APlayerState {
    GENERATED_BODY()

public:
    // Tells the server which attributes need to be replicated on the replicas.
    void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION(BlueprintPure)
    EPlayerGameState GetCurrentState() const { return _currentState; }

    void SetCurrentState(const EPlayerGameState playerGameState) { _currentState = playerGameState; }

    UFUNCTION(BlueprintPure)
    bool IsWinner() const { return _isWinner; }

    // Called by authority, see ATantrumGameStateBase::OnPlayerReachedEnd
    void SetIsWinner(bool isWinner) { _isWinner = isWinner; }

protected:
    UPROPERTY(ReplicatedUsing = OnRep_CurrentState)
    EPlayerGameState _currentState = EPlayerGameState::None;

    UPROPERTY(Replicated)
    bool _isWinner = false;

    UFUNCTION()
    virtual void OnRep_CurrentState(const EPlayerGameState& oldGameState) {}
};
