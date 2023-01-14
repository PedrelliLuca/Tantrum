// Fill out your copyright notice in the Description page of Project Settings.


#include "TantrumCharacterBase.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

ATantrumCharacterBase::ATantrumCharacterBase() {
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Do not rotate when the controller rotates.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate
	
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	// Camera boom creation (pulls in towards the player if there is a collision)
	_cameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	_cameraBoom->SetupAttachment(RootComponent);
	_cameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	_cameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Camera creation
	_followCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	_followCamera->SetupAttachment(_cameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	_followCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

}

void ATantrumCharacterBase::Landed(const FHitResult& hit) {
	Super::Landed(hit);

	if (const auto playerController = Cast<APlayerController>(GetController()); IsValid(playerController)) {
		const auto impactVelocity = FMath::Abs(GetVelocity().Z);
		if (impactVelocity < _minStunVelocity) {
			return;
		}

		const auto intensity = FMath::Clamp((impactVelocity - _minStunVelocity) / (_maxStunVelocity - _minStunVelocity), 0.0f, 1.0f);
		const bool bAffectSmall = intensity < 0.5f;
		const bool bAffectLarge = intensity >= 0.5f;

		playerController->PlayDynamicForceFeedback(intensity, 0.5f, bAffectLarge, bAffectSmall, bAffectLarge, bAffectLarge);

		_stunDuration = intensity * (_maxStunDuration - _minStunDuration);
		_unstunnedSpeed = GetCharacterMovement()->MaxWalkSpeed;
		_stunTime = 0.f;
		GetCharacterMovement()->MaxWalkSpeed = 0.f;
	}
}

void ATantrumCharacterBase::Tick(float deltaSeconds) {
	Super::Tick(deltaSeconds);

	if (_stunTime < 0.f) {
		return;
	}

	_stunTime += deltaSeconds;
	if (_stunTime > _stunDuration) {
		GetCharacterMovement()->MaxWalkSpeed = _unstunnedSpeed;
		_stunTime = -1.f;
		return;
	}

	GetCharacterMovement()->MaxWalkSpeed = _unstunnedSpeed * (_stunTime / _stunDuration);
}
