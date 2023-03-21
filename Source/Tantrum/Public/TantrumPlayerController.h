// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "GameFramework/PlayerController.h"
#include "InputMappingContext.h"
#include "TantrumGameStateBase.h"
#include "TantrumGameWidget.h"

#include "TantrumPlayerController.generated.h"

/**
 *
 */
UCLASS()
class TANTRUM_API ATantrumPlayerController : public APlayerController {
    GENERATED_BODY()

public:
    UFUNCTION(Client, Reliable)
    void ClientDisplayCountdown(float gameCountdownDuration, TSubclassOf<UTantrumGameWidget> gameWidgetClass);

    UFUNCTION(Client, Reliable)
    void ClientRestartGame();

    UFUNCTION(Client, Reliable)
    void ClientReachedEnd();

    UFUNCTION(BlueprintCallable)
    void OnRetrySelected();

protected:
    void BeginPlay() override;

    void SetupInputComponent() override;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputMappingContext> _defaultMappingContext;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputAction> _jumpAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputAction> _moveAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> _lookAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> _sprintAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> _crouchAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> _pullAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> _throwAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    float _flickThreshold = 0.75f;

    void ReceivedPlayer() override;

private:
    bool _canProcessRequest() const;

    void _jump();
    void _stopJumping();

    void _move(const FInputActionValue& value);

    void _look(const FInputActionValue& value);

    void _sprintTriggered();
    void _sprintCanceled();

    void _crouchTriggered();
    void _crouchCanceled();

    void _requestPullOrAim();
    void _requestStopPullOrAim();

    void _throw(const FInputActionValue& value);

    UFUNCTION(Server, Reliable)
    void _serverRestartLevel();

    float _lastThrowAxis = 0;

    UPROPERTY(EditAnywhere, Category = "HUD")
    TSubclassOf<UUserWidget> _hudClass = nullptr;

    UPROPERTY()
    TObjectPtr<UUserWidget> _hudWidget;

    TWeakObjectPtr<ATantrumGameStateBase> _tantrumGameState;

    UPROPERTY()
    TObjectPtr<UTantrumGameWidget> _tantrumGameWidget = nullptr;
};
