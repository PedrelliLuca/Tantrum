// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "GameFramework/PlayerController.h"
#include "InputMappingContext.h"

#include "TantrumPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class TANTRUM_API ATantrumPlayerController : public APlayerController {
	GENERATED_BODY()

public:
	ATantrumPlayerController();

	void Tick(float deltaSeconds) override;
	
protected:
	void BeginPlay() override;
	
	void SetupInputComponent() override;

	UPROPERTY(EditAnywhere, Category=Input)
	TObjectPtr<UInputMappingContext> _defaultMappingContext;

	UPROPERTY(EditAnywhere, Category=Input)
	TObjectPtr<UInputAction> _jumpAction;

	UPROPERTY(EditAnywhere, Category=Input)
	TObjectPtr<UInputAction> _moveAction;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputAction> _lookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputAction> _sprintAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputAction> _crouchAction;

	UPROPERTY(EditAnywhere, Category = Input)
	float _sprintSpeed = 1200.0f;

	UPROPERTY(EditAnywhere, Category = Input)
	float _walkSpeed = 600.0f;

	// The final velocity for falling from 3m
	UPROPERTY(EditAnywhere, Category = "Stun")
	float _minStunVelocity = 800.f;

	UPROPERTY(EditAnywhere, Category = "Stun")
	float _maxStunVelocity = 1600.f;

	UPROPERTY(EditAnywhere, Category = "Stun")
	float _maxStunDuration = 10.f;

	UPROPERTY(EditAnywhere, Category = "Stun")
	float _minStunDuration = 1.f;

private:
	void _jump();
	void _stopJumping();

	UFUNCTION()
	void _onLanded(const FHitResult& hit);
	
	void _move(const FInputActionValue& value);

	void _look(const FInputActionValue& value);

	void _sprintTriggered();
	void _sprintCanceled();

	void _crouchTriggered();
	void _crouchCanceled();

	float _stunDuration = 0.f;

	float _stunTime = -1.f;
};
