// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "GameFramework/GameStateBase.h"
#include "TantrumCharacterBase.h"

#include "TantrumGameStateBase.generated.h"

UENUM(BlueprintType)
enum class EGameState : uint8
{
    None UMETA(DisplayName = "None"),
    Waiting UMETA(DisplayName = "Waiting"),
    Playing UMETA(DisplayName = "Playing"),
    Paused UMETA(DisplayName = "Paused"),
    GameOver UMETA(DisplayName = "Game Over")
};

USTRUCT()
struct FGameResult {
    GENERATED_BODY()

public:
    UPROPERTY()
    FString Name;

    UPROPERTY()
    float Time;
};

/**
 *
 */
UCLASS()
class TANTRUM_API ATantrumGameStateBase : public AGameStateBase {
    GENERATED_BODY()

public:
    // Tells the server which attributes need to be replicated on the replicas.
    void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION(BlueprintCallable)
    void SetGameState(const EGameState gameState) { _gameState = gameState; }

    UFUNCTION(BlueprintPure)
    EGameState GetGameState() const { return _gameState; }

    UFUNCTION(BlueprintPure)
    bool IsPlaying() const { return _gameState == EGameState::Playing; }

    // Will only be called on authority
    void OnPlayerReachedEnd(ATantrumCharacterBase* tantrumCharacter);

    UFUNCTION()
    void ClearResults() { _results.Empty(); }

protected:
    UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_GameState, Category = "States")
    EGameState _gameState = EGameState::None;

    UFUNCTION()
    void OnRep_GameState(const EGameState& oldGameState);

    UPROPERTY(VisibleAnywhere, Replicated, Category = "States")
    TArray<FGameResult> _results;
};
