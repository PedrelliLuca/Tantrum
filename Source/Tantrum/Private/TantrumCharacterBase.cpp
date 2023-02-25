// Fill out your copyright notice in the Description page of Project Settings.


#include "TantrumCharacterBase.h"

#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"

constexpr int CVSphereCastPlayerView = 0;
constexpr int CVSphereCastActorTransform = 1;
constexpr int CVLineCastActorTransform = 2;

static TAutoConsoleVariable<int> CVarTraceMode(
	TEXT("Tantrum.Character.Debug.TraceMode"),
	0,
	TEXT("    0: Sphere cast PlayerView is used for direction/rotation (default).\n")
	TEXT("    1: Sphere cast using ActorTransform \n")
	TEXT("    2: Line cast using ActorTransform \n"),
	ECVF_Default
);

static TAutoConsoleVariable<bool> CVarDisplayTrace(
	TEXT("Tantrum.Character.Debug.DisplayTrace"),
	false,
	TEXT("Display Trace\n"),
	ECVF_Default
);

static TAutoConsoleVariable<bool> CVarDisplayThrowVelocity(
	TEXT("Tantrum.Character.Debug.DisplayThrowVelocity"),
	false,
	TEXT("Display Throw Velocity\n"),
	ECVF_Default
);

ATantrumCharacterBase::ATantrumCharacterBase() {
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true; // Replicating the player
	SetReplicateMovement(true); // Character movement replication

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

void ATantrumCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams sharedParams;
	sharedParams.bIsPushBased = true;
	sharedParams.Condition = COND_SkipOwner; // The machine that sent the replication request sets the new value locally, so there is no point in having the server sending it back.

	DOREPLIFETIME_WITH_PARAMS_FAST(ATantrumCharacterBase, _characterThrowState, sharedParams);
}

void ATantrumCharacterBase::Landed(const FHitResult& hit) {
	const auto impactVelocity = FMath::Abs(GetVelocity().Z);
	if (impactVelocity < _minStunVelocity) {
		return;
	}

	const auto intensity = FMath::Clamp((impactVelocity - _minStunVelocity) / (_maxStunVelocity - _minStunVelocity), 0.0f, 1.0f);
	const bool bAffectSmall = intensity < 0.5f;
	const bool bAffectLarge = intensity >= 0.5f;

	if (const auto playerController = Cast<APlayerController>(GetController())) {
		playerController->PlayDynamicForceFeedback(intensity, 0.5f, bAffectLarge, bAffectSmall, bAffectLarge, bAffectLarge);
	}

	_stunDuration = intensity * (_maxStunDuration - _minStunDuration);
	_stunTime = 0.f;
	GetCharacterMovement()->MaxWalkSpeed = 0.f;
}

void ATantrumCharacterBase::RequestSprint() {
	// Can't sprint while stunned
	if (_isStunned()) {
		return;
	}

	GetCharacterMovement()->MaxWalkSpeed = _sprintSpeed;
}

void ATantrumCharacterBase::RequestSprintCancelation() {
	// Can't sprint while stunned
	if (_isStunned()) {
		return;
	}

	GetCharacterMovement()->MaxWalkSpeed = _walkSpeed;
}

bool ATantrumCharacterBase::IsPullingObject() const {
	return _characterThrowState == ECharacterThrowState::RequestingPull || _characterThrowState == ECharacterThrowState::Pulling;
}

void ATantrumCharacterBase::RequestPull() {
	if (!_isStunned() && _characterThrowState == ECharacterThrowState::None) {
		_characterThrowState = ECharacterThrowState::RequestingPull;
	}
}

void ATantrumCharacterBase::RequestPullCancelation() {
	// This makes the animation of the pull stop via IsPullingObject()
	if (_characterThrowState == ECharacterThrowState::RequestingPull) {
		_characterThrowState = ECharacterThrowState::None;
	}
}

void ATantrumCharacterBase::RequestUseObject() {
	ApplyEffect_Implementation(_throwable->GetEffectType(), true);
	_throwable->Destroy();
	_resetThrowableObject();
}


bool ATantrumCharacterBase::CanThrow() const {
	return _characterThrowState == ECharacterThrowState::Attached;
}

void ATantrumCharacterBase::RequestThrow() {
	if (!CanThrow()) {
		return;
	}

	// To give a responsive feel on the local machine, play locally on the owned actor.
	if (_playThrowMontage()) {
		_characterThrowState = ECharacterThrowState::Throwing;
		// Client telling the server "hey, I'm playing this animation, could you please tell replicas on other machines?"
		_serverRequestThrowObject();
	} else {
		_resetThrowableObject();
	}
}

void ATantrumCharacterBase::_serverRequestThrowObject_Implementation() {
	// Server receives the request from one of the clients' "RequestThrow()" function
	// Servers says "Alright, I'll send this to anybody including both the sender and myself.
	_multicastRequestThrowObject();
}

void ATantrumCharacterBase::_multicastRequestThrowObject_Implementation() {
	// The original sender has already played the montage in "RequestThrow()", we don't want they to play it again.
	if (IsLocallyControlled()) {
		return;
	}

	_playThrowMontage();
	// This doesn't have to be here, we're already replicating the property, see header.
	_characterThrowState = ECharacterThrowState::Throwing;
}

void ATantrumCharacterBase::Tick(const float deltaSeconds) {
	Super::Tick(deltaSeconds);

	/* The replica does not need to concern itself with trying to throw an object and doing raycasts for objects. 
	 * Consider you have a 4 player game, and let's consider player #2 for example.
	 * The character of player 2 is replicated 3 times on machines 1, 3, and 4. However, only the character on machine #2 will be able, through the player controller, to pull and throw objects,
	 * because the actual player #2, on their machine, will do so. The characters on machines 1, 3 and 4, being replicas, do not need to execute the logic for pulling and throwing, they just need
	 * to show players #1, #3, and #4 that player #2, on their machine, is doing so.
	 */
	if (!IsLocallyControlled()) {
		return;
	}

	_updateStun(deltaSeconds);
	if (_isStunned()) {
		return;
	} 

	if (_bIsUnderEffect) {
		if (_effectCooldown > 0.0f) {
			_effectCooldown -= deltaSeconds;
		} else {
			_bIsUnderEffect = false;
			_effectCooldown = _defaultEffectCooldown;
			EndEffect();
		}
	}

	if (_characterThrowState == ECharacterThrowState::Throwing) {
		if (const auto animInstance = GetMesh()->GetAnimInstance()) {
			if (const auto animMontage = animInstance->GetCurrentActiveMontage()) {
				// The montage speed is determined by the curve in the uasset. By speeding up on certain parts, the fact that the
				// character is throwing an object is more apparent.
				const float playRate = animInstance->GetCurveValue(TEXT("ThrowCurve"));
				animInstance->Montage_SetPlayRate(animMontage, playRate);
			}
		}
	} else if (_characterThrowState == ECharacterThrowState::None || _characterThrowState == ECharacterThrowState::RequestingPull) {
		// We're in a state that allows to pick up objects.
		// How are we tracing for _throwables?
		switch (CVarTraceMode->GetInt()) {
		case CVSphereCastPlayerView:
			// Sphere trace from the camera (best option by far)
			_sphereCastPlayerView();
			break;
		case CVSphereCastActorTransform:
			// Sphere trace from the character
			_sphereCastActorTransform();
			break;
		case CVLineCastActorTransform:
			// Line trace from the character
			_lineCastActorTransform();
			break;
		default:
			_sphereCastPlayerView();
		}
	}
}

void ATantrumCharacterBase::OnRep_CharacterThrowState(const ECharacterThrowState& oldCharacterThrowState) {
	if (_characterThrowState != oldCharacterThrowState) {
		UE_LOG(LogTemp, Warning, TEXT("%s(): OldThrowState: %s"), *FString{__FUNCTION__}, *UEnum::GetDisplayValueAsText(oldCharacterThrowState).ToString());
		UE_LOG(LogTemp, Warning, TEXT("%s(): CharacterThrowState: %s"), *FString{__FUNCTION__}, *UEnum::GetDisplayValueAsText(_characterThrowState).ToString());
	}
}

void ATantrumCharacterBase::ApplyEffect_Implementation(EEffectType effectType, bool bIsBuff) {
	if (_bIsUnderEffect) {
		return;
	}

	_currentEffect = effectType;
	_bIsUnderEffect = true;
	_bIsEffectBuff = bIsBuff;

	switch (_currentEffect) {
	case EEffectType::Speed:
		_bIsEffectBuff ? _sprintSpeed *= 2.0f : GetCharacterMovement()->DisableMovement();
	default:
		break;
	}
}

void ATantrumCharacterBase::EndEffect() {
	_bIsUnderEffect = false;
	switch (_currentEffect) {
	case EEffectType::Speed:
		_bIsEffectBuff ? _sprintSpeed *= 0.5f, RequestSprintCancelation() : GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	default:
		break;
	}
}

void ATantrumCharacterBase::BeginPlay() {
	Super::BeginPlay();

	_effectCooldown = _defaultEffectCooldown;
}

void ATantrumCharacterBase::_updateStun(const float deltaSeconds) {
	if (_stunTime < 0.f) {
		return;
	}

	_stunTime += deltaSeconds;
	if (_stunTime > _stunDuration) {
		GetCharacterMovement()->MaxWalkSpeed = _walkSpeed;
		_stunTime = -1.f;
		return;
	}

	GetCharacterMovement()->MaxWalkSpeed = _walkSpeed * (_stunTime / _stunDuration);
}

bool ATantrumCharacterBase::_playThrowMontage() {
	const float playRate = 1.0f;
	const bool bPlayedSuccessfully = PlayAnimMontage(_throwMontage, playRate) > 0.0f;
	
	// We don't want replicas to bind to these delegates
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
}

void ATantrumCharacterBase::_sphereCastPlayerView() {
	FVector location;
	FRotator rotation;

	const auto controller = GetController();
	check(IsValid(controller)); // This should always be valid since we're getting here only on the server.
	controller->GetPlayerViewPoint(location, rotation);

	const auto playerViewForward = rotation.Vector();
	const auto additionalDistance = (location - GetActorLocation()).Size(); // player to camera distance
	const auto endLocation = location + (playerViewForward * (_pullRange + additionalDistance));

	const auto forwardVector = GetActorForwardVector();
	const auto dotProduct = FVector::DotProduct(playerViewForward, forwardVector);

	// Prevent picking up objects behind the character. This happens when the pull input is given with the camera looking
	// at the character's front side.
	if (dotProduct < -0.23f) {
		if (_throwable.IsValid()) {
			_throwable->ToggleHighlight(false);
			_throwable = nullptr;
		}
		return;
	}

	FHitResult hitResult;
	const auto debugTrace = CVarDisplayTrace->GetBool() ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None;

	TArray<AActor*> actorsToIgnore;
	actorsToIgnore.Add(this);

	// Same function as the BP node
	UKismetSystemLibrary::SphereTraceSingle(GetWorld(), location, endLocation, _pullSphereTraceRadius, UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility), false, actorsToIgnore, debugTrace, hitResult, true);
	_processTraceResult(hitResult);

#if ENABLE_DRAW_DEBUG
	if (CVarDisplayTrace->GetBool()) {
		static float fovDeg = 90.0f;
		DrawDebugCamera(GetWorld(), location, rotation, fovDeg);
		DrawDebugLine(GetWorld(), location, endLocation, hitResult.bBlockingHit ? FColor::Red : FColor::White);
		DrawDebugPoint(GetWorld(), endLocation, _debugPointRadius, hitResult.bBlockingHit ? FColor::Red : FColor::White);

	}
#endif
}

void ATantrumCharacterBase::_sphereCastActorTransform() {
	const auto startLoc = GetActorLocation();
	const auto endLoc = startLoc + (GetActorForwardVector() * _pullRange);

	const auto debugTrace = CVarDisplayTrace->GetBool() ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None;
	FHitResult hitResult;

	UKismetSystemLibrary::SphereTraceSingle(GetWorld(), startLoc, endLoc, _debugPointRadius, UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility), false, TArray<AActor*>{}, debugTrace, hitResult, true);
	_processTraceResult(hitResult);
}

void ATantrumCharacterBase::_lineCastActorTransform() {
	const auto startLoc = GetActorLocation();
	const auto endLoc = startLoc + (GetActorForwardVector() * _pullRange);
	
	FHitResult hitResult;
	GetWorld()->LineTraceSingleByChannel(hitResult, startLoc, endLoc, ECollisionChannel::ECC_Visibility);
	_processTraceResult(hitResult);

#if ENABLE_DRAW_DEBUG
	if (CVarDisplayTrace->GetBool()) {
		DrawDebugLine(GetWorld(), startLoc, endLoc, hitResult.bBlockingHit ? FColor::Red : FColor::White);
	}
#endif
}

void ATantrumCharacterBase::_processTraceResult(const FHitResult& hitResult) {
	const auto hitThrowable = Cast<AThrowable>(hitResult.GetActor());

	const bool isSameActor = _throwable == hitThrowable;
	const bool isValidTarget = IsValid(hitThrowable) && hitThrowable->IsIdle();

	// Clean up of old _throwable
	if (_throwable.IsValid()) {
		if (!isValidTarget || !isSameActor) {
			_throwable->ToggleHighlight(false);
			_throwable = nullptr;
		}
	}

	if (isValidTarget) {
		if (!_throwable.IsValid()) {
			_setThrowable(hitThrowable);

			_throwable->ToggleHighlight(true);
		}
	}

	if (_characterThrowState == ECharacterThrowState::RequestingPull) {
		// You can't pull while sprinting
		if (FMath::IsNearlyEqual(GetCharacterMovement()->MaxWalkSpeed, _sprintSpeed) || GetCharacterMovement()->MaxWalkSpeed > _sprintSpeed) {
			return;
		}

		if (_throwable.IsValid() && _throwable->Pull(this)) {
			_characterThrowState = ECharacterThrowState::Pulling;
		}
	}
}

void ATantrumCharacterBase::_onMontageBlendingOut(UAnimMontage* montage, bool bInterrupted) {
}

void ATantrumCharacterBase::_onMontageEnded(UAnimMontage* montage, bool bInterrupted) {
	_unbindMontage();
	_characterThrowState = ECharacterThrowState::None;
	MoveIgnoreActorRemove(_throwable.Get());

	if (_throwable->GetRootComponent()) {
		if (const auto throwableRoot = Cast<UPrimitiveComponent>(_throwable->GetRootComponent())) {
			throwableRoot->IgnoreActorWhenMoving(this, false);
		}
	}

	_resetThrowableObject();
}

void ATantrumCharacterBase::_unbindMontage() {
	if (const auto animInstance = GetMesh()->GetAnimInstance()) {
		animInstance->OnPlayMontageNotifyBegin.RemoveDynamic(this, &ATantrumCharacterBase::_onNotifyBeginReceived);
		animInstance->OnPlayMontageNotifyEnd.RemoveDynamic(this, &ATantrumCharacterBase::_onNotifyEndReceived);
	}
}

void ATantrumCharacterBase::_setThrowable(TWeakObjectPtr<AThrowable> newThrowable) {
	check(newThrowable.IsValid());

	_throwable = MoveTemp(newThrowable);
	_throwable->OnThrowableAttached().AddUObject(this, &ATantrumCharacterBase::_onThrowableAttached);
	_throwable->OnThrowableMissed().AddUObject(this, &ATantrumCharacterBase::_resetThrowableObject);
}

void ATantrumCharacterBase::_resetThrowableObject() {
	_characterThrowState = ECharacterThrowState::None;

	if (_throwable.IsValid()) {
		_throwable->Drop();
		_throwable->OnThrowableAttached().RemoveAll(this);
		_throwable->OnThrowableMissed().RemoveAll(this);
	}
	_throwable = nullptr;
}

void ATantrumCharacterBase::_onThrowableAttached() {
	check(_throwable.IsValid());
	_characterThrowState = ECharacterThrowState::Attached;
	MoveIgnoreActorAdd(_throwable.Get());
}

void ATantrumCharacterBase::_onNotifyBeginReceived(FName notifyName, const FBranchingPointNotifyPayload& branchingPointNotifyPayload) {
	if (notifyName != TEXT("ThrowNotify")) {
		return;
	}

	if (_throwable->GetRootComponent()) {
		if (const auto throwableRoot = Cast<UPrimitiveComponent>(_throwable->GetRootComponent())) {
			throwableRoot->IgnoreActorWhenMoving(this, true);
		}
	}

	const auto throwDirection = GetActorForwardVector() * _throwSpeed;
	_throwable->Throw(throwDirection);

	if (CVarDisplayThrowVelocity->GetBool()) {
		const auto start = GetMesh()->GetSocketLocation(TEXT("ObjectAttach"));
		DrawDebugLine(GetWorld(), start, start + throwDirection, FColor::Red, false, 5.0f);
	}
}

void ATantrumCharacterBase::_onNotifyEndReceived(FName notifyName, const FBranchingPointNotifyPayload& branchingPointNotifyPayload) {
}
