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

ATantrumPlayerController::ATantrumPlayerController() {
}

void ATantrumPlayerController::Tick(float deltaSeconds) {
	Super::Tick(deltaSeconds);

	if (_stunTime < 0.f) {
		return;
	}

	_stunTime += deltaSeconds;
	if (_stunTime > _stunDuration) {
		GetCharacter()->GetCharacterMovement()->MaxWalkSpeed = _walkSpeed;
		_stunTime = -1.f;
		return;
	}

	GetCharacter()->GetCharacterMovement()->MaxWalkSpeed = _walkSpeed * (_stunTime / _stunDuration);
}

void ATantrumPlayerController::BeginPlay() {
	Super::BeginPlay();

	// Set up the mapping context
	if (const auto subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer())) {
		// The 0 priority will make the _defaultMappingContext easily overridable by other contexts
		subsystem->AddMappingContext(_defaultMappingContext, 0);
	}

	GetCharacter()->LandedDelegate.AddDynamic(this, &ATantrumPlayerController::_onLanded);
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

void ATantrumPlayerController::_onLanded(const FHitResult& hit) {
	const auto impactVelocity = FMath::Abs(GetCharacter()->GetVelocity().Z);
	if (impactVelocity < _minStunVelocity) {
		return;
	}

	const auto intensity = FMath::Clamp((impactVelocity - _minStunVelocity) / (_maxStunVelocity - _minStunVelocity), 0.0f, 1.0f);
	const bool bAffectSmall = intensity < 0.5f;
	const bool bAffectLarge = intensity >= 0.5f;

	PlayDynamicForceFeedback(intensity, 0.5f, bAffectLarge, bAffectSmall, bAffectLarge, bAffectLarge);

	_stunDuration = intensity * (_maxStunDuration - _minStunDuration);
	_stunTime = 0.f;
	GetCharacter()->GetCharacterMovement()->MaxWalkSpeed = 0.f;
}

void ATantrumPlayerController::_move(const FInputActionValue& value) {
	const auto movementVector = value.Get<FVector2D>();

	const auto pawn = GetPawn();
	check(IsValid(pawn));
	
	pawn->AddMovementInput(pawn->GetActorForwardVector(), movementVector.Y);
	pawn->AddMovementInput(pawn->GetActorRightVector(), movementVector.X);
}

void ATantrumPlayerController::_look(const FInputActionValue& value) {
	const auto lookAxisVector = value.Get<FVector2D>();

	const auto pawn = GetPawn();
	check(IsValid(pawn));
	
	pawn->AddControllerYawInput(lookAxisVector.X);
	pawn->AddControllerPitchInput(lookAxisVector.Y);
}

void ATantrumPlayerController::_sprintTriggered() {
	// Can't sprint while stunned
	if (_stunTime > 0.f) {
		return;
	}

	const auto character = GetCharacter();
	check(IsValid(character));

	character->GetCharacterMovement()->MaxWalkSpeed = _sprintSpeed;
}

void ATantrumPlayerController::_sprintCanceled() {
	// Can't sprint while stunned
	if (_stunTime > 0.f) {
		return;
	}

	const auto character = GetCharacter();
	check(IsValid(character));

	character->GetCharacterMovement()->MaxWalkSpeed = _walkSpeed;
}

void ATantrumPlayerController::_crouchTriggered() {
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
				UE_LOG(LogTemp, Warning, TEXT("Axis: %f currentDelta %f"), throwAxis, _lastThrowAxis);
			}
		}

		_lastThrowAxis = throwAxis;

		if (delta > _flickThreshold) {
			tantrumChar->RequestThrow();
		}
	}
}
