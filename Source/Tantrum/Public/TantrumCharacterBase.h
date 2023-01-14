// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"

#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"

#include "TantrumCharacterBase.generated.h"

UCLASS()
class TANTRUM_API ATantrumCharacterBase : public ACharacter {
	GENERATED_BODY()

public:
	ATantrumCharacterBase();

	void Landed(const FHitResult& hit) override;

	void Tick(float deltaSeconds) override;

protected:
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, Category = Camera)
	TObjectPtr<USpringArmComponent> _cameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, Category = Camera)
	TObjectPtr<UCameraComponent> _followCamera;

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
	float _stunDuration = 0.f;

	float _stunTime = -1.f;
	float _unstunnedSpeed = 0.f;
};
