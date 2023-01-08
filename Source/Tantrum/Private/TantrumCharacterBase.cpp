// Fill out your copyright notice in the Description page of Project Settings.


#include "TantrumCharacterBase.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

ATantrumCharacterBase::ATantrumCharacterBase() {
}

void ATantrumCharacterBase::BeginPlay() {
	Super::BeginPlay();

	if (APlayerController* playerController = Cast<APlayerController>(Controller)) {
		if (UEnhancedInputLocalPlayerSubsystem* subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(playerController->GetLocalPlayer())) {
			subsystem->AddMappingContext(_defaultMappingContext, 0);
		}
	}
}

void ATantrumCharacterBase::SetupPlayerInputComponent(UInputComponent* playerInputComponent) {
	Super::SetupPlayerInputComponent(playerInputComponent);

	// Set up action bindings
	const auto enhancedInputComponent = CastChecked<UEnhancedInputComponent>(playerInputComponent);
	
	enhancedInputComponent->BindAction(_jumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
	enhancedInputComponent->BindAction(_jumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

	enhancedInputComponent->BindAction(_moveAction, ETriggerEvent::Triggered, this, &ATantrumCharacterBase::_move);

	enhancedInputComponent->BindAction(_lookAction, ETriggerEvent::Triggered, this, &ATantrumCharacterBase::_look);
}

void ATantrumCharacterBase::_move(const FInputActionValue& value) {
	const auto movementVector = value.Get<FVector2D>();

	if (IsValid(Controller)) {
		AddMovementInput(GetActorForwardVector(), movementVector.Y);
		AddMovementInput(GetActorRightVector(), movementVector.X);
	}
}

void ATantrumCharacterBase::_look(const FInputActionValue& value) {
	const auto lookAxisVector = value.Get<FVector2D>();

	if (IsValid(Controller)) {
		AddControllerYawInput(lookAxisVector.X);
		AddControllerPitchInput(lookAxisVector.Y);
	}
}
