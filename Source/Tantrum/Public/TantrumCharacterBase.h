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

protected:
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, Category = Camera)
	TObjectPtr<USpringArmComponent> _cameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, Category = Camera)
	TObjectPtr<UCameraComponent> _followCamera;
};
