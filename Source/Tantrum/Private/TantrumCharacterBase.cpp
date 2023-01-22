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

	_throwAbilityC = CreateDefaultSubobject<UThrowAbilityComponent>(TEXT("ThrowAbility"));
}

void ATantrumCharacterBase::RequestPull() {
	/*if (!bIsStunned && _characterThrowState == ECharacterThrowState::None) {
		_characterThrowState = ECharacterThrowState::RequestingPull;
	}*/

	// You can't pull while sprinting or while having a throwable ready to be thrown
	/*if (GetOwner()->GetVelocity().SizeSquared() >= 100.0f) {
		return;
	}

	if (_throwable.IsValid() && _throwable->Pull(this)) {
		_characterThrowState = ECharacterThrowState::Pulling;
		_throwable = nullptr;
	}*/
}

void ATantrumCharacterBase::RequestPullCancelation() {
}


bool ATantrumCharacterBase::CanThrow() const {
	return true;
}

void ATantrumCharacterBase::RequestThrow() {
	if (!CanThrow()) {
		return;
	}

	/*if (_playThrowMontage()) {
		_characterThrowState = ECharacterThrowState::Throwing;
	} else {
		_resetThrowable();
	}*/

	const auto throwableRoot = Cast<UPrimitiveComponent>(_throwable->GetRootComponent());
	check(IsValid(throwableRoot));
	throwableRoot->IgnoreActorWhenMoving(GetOwner(), true);

	const auto throwDirection = GetOwner()->GetActorForwardVector() * _throwSpeed;
	_throwable->Throw(throwDirection);
}

void ATantrumCharacterBase::Tick(float deltaSeconds) {
	Super::Tick(deltaSeconds);

	// _updateStun();
	/*if (_bIsStunned) {
		return;
	}

	if (_characterThrowState == ECharacterThrowState::Throwing) {
		if (const auto animInstance = GetMesh()->GetAnimInstance()) {
			if (const auto animMontage = animInstance->GetCurrentActiveMontage()) {
				const float playRate = animInstance->GetCurveValue(TEXT("ThrowCurve"));
				animInstance->Montage_SetPlayRate(animMontage, playRate);
			}
		}
	}*/
}

bool ATantrumCharacterBase::_playThrowMontage() {
	const float playRate = 1.0f;
	const bool bPlayedSuccessfully = PlayAnimMontage(_throwMontage, playRate) > 0.0f;
	
	if (bPlayedSuccessfully) {
		const auto animInstance = GetMesh()->GetAnimInstance();

		// Setting the blending out callback on the montage
		if (!_blendingOutDelegate.IsBound()) {
			_blendingOutDelegate.BindUObject(this, &ATantrumCharacterBase::_onMontageBlendingOut);
		}
		animInstance->Montage_SetBlendingOutDelegate(_blendingOutDelegate, _throwMontage);

		// Setting the end delegate callback on the montage
		if (!_montageEndedDelegate.IsBound()) {
			_montageEndedDelegate.BindUObject(this, &ATantrumCharacterBase::_onMontageEnded);
		}
		animInstance->Montage_SetEndDelegate(_montageEndedDelegate, _throwMontage);

		// Set the callbacks for when the montage begins and ends on the character
		animInstance->OnPlayMontageNotifyBegin.AddDynamic(this, &ATantrumCharacterBase::_onNotifyBeginReceived);
		animInstance->OnPlayMontageNotifyEnd.AddDynamic(this, &ATantrumCharacterBase::_onNotifyEndReceived);
	}

	return bPlayedSuccessfully;


	//return true;
}

void ATantrumCharacterBase::_resetThrowable() {
}

void ATantrumCharacterBase::_onMontageBlendingOut(UAnimMontage* montage, bool bInterrupted) {
}

void ATantrumCharacterBase::_onMontageEnded(UAnimMontage* montage, bool bInterrupted) {
	_unbindMontage();
	// _characterThrowState = ECharacterThrowState::None;
	MoveIgnoreActorRemove(_throwable.Get());

	if (_throwable->GetRootComponent()) {
		if (const auto throwableRoot = Cast<UPrimitiveComponent>(_throwable->GetRootComponent())) {
			throwableRoot->IgnoreActorWhenMoving(this, false);
		}
	}

	_throwable = nullptr;
}

void ATantrumCharacterBase::_unbindMontage() {
}

void ATantrumCharacterBase::_onNotifyBeginReceived(FName notifyName, const FBranchingPointNotifyPayload& branchingPointNotifyPayload) {
	if (!(notifyName == "ThrowNotify")) {
		return;
	}

	if (_throwable->GetRootComponent()) {
		if (const auto throwableRoot = Cast<UPrimitiveComponent>(_throwable->GetRootComponent())) {
			throwableRoot->IgnoreActorWhenMoving(this, true);
		}

		const auto throwDirection = GetActorForwardVector() * _throwSpeed;
		_throwable->Throw(throwDirection);
	}
}

void ATantrumCharacterBase::_onNotifyEndReceived(FName notifyName, const FBranchingPointNotifyPayload& branchingPointNotifyPayload) {
}
