// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TantrumGameModeBase.generated.h"

UENUM(BlueprintType)
enum class EGameState : uint8 {
	None     UMETA(DisplayName = "None"),
	Waiting  UMETA(DisplayName = "Waiting"),
	Playing  UMETA(DisplayName = "Playing"),
	Paused   UMETA(DisplayName = "Paused"),
	GameOver UMETA(DisplayName = "Game Over"),
};

/**
 * 
 */
UCLASS()
class TANTRUM_API ATantrumGameModeBase : public AGameModeBase {
	GENERATED_BODY()

public:
	EGameState GetCurrentGameState() const;
	void PlayerReachedEnd();

protected:
	void BeginPlay() override;

private:
	void _displayCountdown();
	void _startGame();

	UPROPERTY(VisibleAnywhere)
	EGameState _gameState = EGameState::None;
	
	UPROPERTY(EditAnywhere)
	float _gameCountdownDuration = 2.0f;
};
