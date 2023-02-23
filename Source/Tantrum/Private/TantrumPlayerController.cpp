// Fill out your copyright notice in the Description page of Project Settings.


#include "TantrumPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TantrumCharacterBase.h"

static TAutoConsoleVariable<bool> CVarDisplayLaunchInputDelta(
	TEXT("Tantrum.Character.Debug.DisplayLaunchInputDelta"),
	false,
	TEXT("Display Launch Input Delta"),
	ECVF_Default
);


void ATantrumPlayerController::BeginPlay() {
	Super::BeginPlay();

	// Set up the mapping context
	if (const auto subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer())) {
		// The 0 priority will make the _defaultMappingContext easily overridable by other contexts
		subsystem->AddMappingContext(_defaultMappingContext, 0);
	}

	_gameMode = Cast<ATantrumGameModeBase>(GetWorld()->GetAuthGameMode());
	check(_gameMode.IsValid());
}

void ATantrumPlayerController::SetupInputComponent() {
	Super::SetupInputComponent();

	// Set up bindings to input actions. This won't have any effect unless a mapping context has been added to the local
	// player subsystem
	const auto enhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);

	check(IsValid(_jumpAction));
	// Cannot bind to ACharacter::Jump() here because GetCharacter() returns nullptr when SetupInputComponent() is called
	enhancedInputComponent->BindAction(_jumpAction, ETriggerEvent::Triggered, this, &ATantrumPlayerController::_jump);
	enhancedInputComponent->BindAction(_jumpAction, ETriggerEvent::Completed, this, &ATantrumPlayerController::_stopJumping);

	check(IsValid(_moveAction));
	enhancedInputComponent->BindAction(_moveAction, ETriggerEvent::Triggered, this, &ATantrumPlayerController::_move);

	check(IsValid(_lookAction));
	enhancedInputComponent->BindAction(_lookAction, ETriggerEvent::Triggered, this, &ATantrumPlayerController::_look);

	check(IsValid(_sprintAction));
	enhancedInputComponent->BindAction(_sprintAction, ETriggerEvent::Triggered, this, &ATantrumPlayerController::_sprintTriggered);
	enhancedInputComponent->BindAction(_sprintAction, ETriggerEvent::Completed, this, &ATantrumPlayerController::_sprintCanceled);

	check(IsValid(_crouchAction));
	enhancedInputComponent->BindAction(_crouchAction, ETriggerEvent::Triggered, this, &ATantrumPlayerController::_crouchTriggered);
	enhancedInputComponent->BindAction(_crouchAction, ETriggerEvent::Completed, this, &ATantrumPlayerController::_crouchCanceled);

	check(IsValid(_pullAction));
	enhancedInputComponent->BindAction(_pullAction, ETriggerEvent::Triggered, this, &ATantrumPlayerController::_pullTriggered);
	enhancedInputComponent->BindAction(_pullAction, ETriggerEvent::Completed, this, &ATantrumPlayerController::_pullCanceled);

	check(IsValid(_throwAction));
	enhancedInputComponent->BindAction(_throwAction, ETriggerEvent::Triggered, this, &ATantrumPlayerController::_throw);
}

void ATantrumPlayerController::_jump() {
	if (const auto character = GetCharacter(); IsValid(character)) {
		character->Jump();
	}
}

void ATantrumPlayerController::_stopJumping() {
	// Needs to be combo-ed with "Jump Max Hold Time" to work, interrupts the jump, allowing shorter and higher jumps depending on how long the
	// jump input was held.
	if (const auto character = GetCharacter(); IsValid(character)) {
		character->StopJumping();
	}
}

void ATantrumPlayerController::_move(const FInputActionValue& value) {
	check(_gameMode.IsValid());
	if (_gameMode->GetCurrentGameState() != EGameState::Playing) {
		return;
	}

	const auto movementVector = value.Get<FVector2D>();

	/*const auto pawn = GetPawn();
	
	if (IsValid(pawn)) {
		pawn->AddMovementInput(pawn->GetActorForwardVector(), movementVector.Y);
		pawn->AddMovementInput(pawn->GetActorRightVector(), movementVector.X);
	}*/

	const auto pawn = GetPawn();
	
	if (IsValid(pawn)) {
		// This may be different from the pawn's rotation
		const auto rotation = GetControlRotation();
		const auto yawRotation = FRotator{ 0.0, rotation.Yaw, 0.0 };

		const auto forwardDir = FRotationMatrix{ yawRotation }.GetUnitAxis(EAxis::X);
		const auto rightDir = FRotationMatrix{ yawRotation }.GetUnitAxis(EAxis::Y);

		pawn->AddMovementInput(forwardDir, movementVector.Y);
		pawn->AddMovementInput(rightDir, movementVector.X);
	}

	
}

void ATantrumPlayerController::_look(const FInputActionValue& value) {
	// We don't check for the game state here, we want the player to be able to look around even when not playing.

	const auto lookAxisVector = value.Get<FVector2D>();

	const auto pawn = GetPawn();
	if (IsValid(pawn)) {
		pawn->AddControllerYawInput(lookAxisVector.X);
		pawn->AddControllerPitchInput(lookAxisVector.Y);
	}
}

void ATantrumPlayerController::_sprintTriggered() {
	check(_gameMode.IsValid());
	if (_gameMode->GetCurrentGameState() != EGameState::Playing) {
		return;
	}

	if (const auto tantrumChar = Cast<ATantrumCharacterBase>(GetCharacter())) {
		tantrumChar->RequestSprint();
	}
}

void ATantrumPlayerController::_sprintCanceled() {
	if (const auto tantrumChar = Cast<ATantrumCharacterBase>(GetCharacter())) {
		tantrumChar->RequestSprintCancelation();
	}
}

void ATantrumPlayerController::_crouchTriggered() {
	check(_gameMode.IsValid());
	if (_gameMode->GetCurrentGameState() != EGameState::Playing) {
		return;
	}

	const auto character = GetCharacter();
	check(IsValid(character));

	if (!character->GetCharacterMovement()->IsMovingOnGround()) {
		return;
	}

	character->Crouch();
}

void ATantrumPlayerController::_crouchCanceled() {
	const auto character = GetCharacter();
	check(IsValid(character));

	character->UnCrouch();
}

void ATantrumPlayerController::_pullTriggered() {
	check(_gameMode.IsValid());
	if (_gameMode->GetCurrentGameState() != EGameState::Playing) {
		return;
	}

	if (const auto tantrumChar = Cast<ATantrumCharacterBase>(GetCharacter())) {
		tantrumChar->RequestPull();
	}
}

void ATantrumPlayerController::_pullCanceled() {
	if (const auto tantrumChar = Cast<ATantrumCharacterBase>(GetCharacter())) {
		tantrumChar->RequestPullCancelation();
	}
}

void ATantrumPlayerController::_throw(const FInputActionValue& value) {
	check(_gameMode.IsValid());
	if (_gameMode->GetCurrentGameState() != EGameState::Playing) {
		return;
	}

	const auto throwAxis = value.Get<float>();

	if (const auto tantrumChar = Cast<ATantrumCharacterBase>(GetCharacter())) {
		if (!tantrumChar->CanThrow()) {
			_lastThrowAxis = 0.0f;
			return;
		}

		const float delta = throwAxis - _lastThrowAxis;
		//debug
		if (CVarDisplayLaunchInputDelta->GetBool()) {
			if (FMath::Abs(delta) > 0.0f) {
				UE_LOG(LogTemp, Warning, TEXT("Axis: %f currentDelta %f"), throwAxis, delta);
			}
		}

		_lastThrowAxis = throwAxis;

		if (delta > _flickThreshold) {

			if (throwAxis > 0.0f) {
				 tantrumChar->RequestThrow();
			} else {
				tantrumChar->RequestUseObject();
			}

		}
	}
}
