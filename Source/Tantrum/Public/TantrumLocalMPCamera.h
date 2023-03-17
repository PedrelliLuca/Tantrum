// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/SpringArmComponent.h"
#include "TantrumGameModeBase.h"

#include "TantrumLocalMPCamera.generated.h"

UCLASS()
class TANTRUM_API ATantrumLocalMPCamera : public AActor {
    GENERATED_BODY()

public:
    ATantrumLocalMPCamera();

    void Tick(float deltaSeconds) override;

protected:
    void BeginPlay() override;

private:
    UPROPERTY(EditDefaultsOnly, Category = "Player Distance", meta = (ClampMin = "0.0"))
    float _minPlayerDistance = 100.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Player Distance", meta = (ClampMin = "0.0"))
    float _maxPlayerDistance = 1000.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Spring Arm", meta = (ClampMin = "0.0"))
    float _minArmLength = 0.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Spring Arm", meta = (ClampMin = "0.0"))
    float _maxArmLength = 500.0f;

    UPROPERTY(VisibleAnywhere, Category = Camera)
    TObjectPtr<USpringArmComponent> _cameraBoom;

    /** Follow camera */
    UPROPERTY(VisibleAnywhere, Category = Camera)
    TObjectPtr<UCameraComponent> _followCamera;

    TWeakObjectPtr<ATantrumGameModeBase> _tantrumGameMode;
};
