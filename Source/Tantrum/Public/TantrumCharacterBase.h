// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "GameFramework/Character.h"
#include "InputMappingContext.h"

#include "TantrumCharacterBase.generated.h"

UCLASS()
class TANTRUM_API ATantrumCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	ATantrumCharacterBase();

protected:
	void BeginPlay() override;
	
	void SetupPlayerInputComponent(class UInputComponent* playerInputComponent) override;

	/** MappingContext */
	UPROPERTY(EditAnywhere, Category=Input)
	TObjectPtr<UInputMappingContext> _defaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category=Input)
	TObjectPtr<UInputAction> _jumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category=Input)
	TObjectPtr<UInputAction> _moveAction;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputAction> _lookAction;

private:
	void _move(const FInputActionValue& value);

	void _look(const FInputActionValue& value);
};
