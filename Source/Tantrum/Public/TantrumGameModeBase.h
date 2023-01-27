// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TantrumGameWidget.h"

#include "TantrumGameModeBase.generated.h"

UENUM(BlueprintType)
enum class EGameState : uint8 {
	None     UMETA(DisplayName = "None"),
	Waiting  UMETA(DisplayName = "Waiting"),
	Playing  UMETA(DisplayName = "Playing"),
	Paused   UMETA(DisplayName = "Paused"),
	GameOver UMETA(DisplayName = "Game Over")
};

/**
 * 
 */
UCLASS()
class TANTRUM_API ATantrumGameModeBase : public AGameModeBase {
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure)
	EGameState GetCurrentGameState() const;

	void PlayerReachedEnd();

protected:
	void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	EGameState _gameState = EGameState::None;

private:
	void _displayCountdown();
	void _startGame();
	
	UPROPERTY(EditAnywhere)
	float _gameCountdownDuration = 2.0f;

	UPROPERTY()
	TObjectPtr<UTantrumGameWidget> _gameWidget = nullptr;

	UPROPERTY(EditAnywhere, Category = "Widget")
	TSubclassOf<UTantrumGameWidget> _gameWidgetClass = nullptr;

};
