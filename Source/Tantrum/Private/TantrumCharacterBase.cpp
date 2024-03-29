// Fill out your copyright notice in the Description page of Project Settings.

#include "TantrumCharacterBase.h"

#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "TantrumGameInstance.h"
#include "TantrumPlayerState.h"
#include "VisualLogger/VisualLogger.h"

constexpr int CVSphereCastPlayerView = 0;
constexpr int CVSphereCastActorTransform = 1;
constexpr int CVLineCastActorTransform = 2;

static TAutoConsoleVariable<int> CVarTraceMode(TEXT("Tantrum.Character.Debug.TraceMode"), 0,
    TEXT("    0: Sphere cast PlayerView is used for direction/rotation (default).\n") TEXT("    1: Sphere cast using ActorTransform \n")
        TEXT("    2: Line cast using ActorTransform \n"),
    ECVF_Default);

static TAutoConsoleVariable<bool> CVarDisplayTrace(TEXT("Tantrum.Character.Debug.DisplayTrace"), false, TEXT("Display Trace\n"), ECVF_Default);

static TAutoConsoleVariable<bool> CVarDisplayThrowVelocity(
    TEXT("Tantrum.Character.Debug.DisplayThrowVelocity"), false, TEXT("Display Throw Velocity\n"), ECVF_Default);

DEFINE_LOG_CATEGORY_STATIC(LogTantrumChar, Verbose, Verbose);

ATantrumCharacterBase::ATantrumCharacterBase() {
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true; // Replicating the player
    SetReplicateMovement(true);

    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

    // Do not rotate when the controller rotates.
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // Configure character movement
    GetCharacterMovement()->bOrientRotationToMovement = true;            // Character moves in the direction of input...
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
    _cameraBoom->TargetArmLength = 400.0f;       // The camera follows at this distance behind the character
    _cameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

    // Camera creation
    _followCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    _followCamera->SetupAttachment(
        _cameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
    _followCamera->bUsePawnControlRotation = false;    // Camera does not rotate relative to arm
}

void ATantrumCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    FDoRepLifetimeParams sharedParams;
    sharedParams.bIsPushBased = true;
    sharedParams.Condition =
        COND_SkipOwner; // The machine that sent the replication request sets the new value locally, so there is no point in having the server sending it back.

    DOREPLIFETIME_WITH_PARAMS_FAST(ATantrumCharacterBase, _characterThrowState, sharedParams);

    sharedParams.Condition = COND_None;
    DOREPLIFETIME_WITH_PARAMS_FAST(ATantrumCharacterBase, _isBeingRescued, sharedParams);
    DOREPLIFETIME_WITH_PARAMS_FAST(ATantrumCharacterBase, _lastGroundPosition, sharedParams);
    DOREPLIFETIME_WITH_PARAMS_FAST(ATantrumCharacterBase, _isStunned, sharedParams);
    DOREPLIFETIME_WITH_PARAMS_FAST(ATantrumCharacterBase, _bIsUnderEffect, sharedParams);
}

void ATantrumCharacterBase::Landed(const FHitResult& hit) {
    Super::Landed(hit);
    const auto netModeString = _getNetModeDebugString();
    UE_LOG(LogTemp, Warning, TEXT("%s() called in net mode %s"), *FString{__FUNCTION__}, *netModeString);

    const auto impactVelocity = FMath::Abs(GetVelocity().Z);
    if (impactVelocity < _minStunVelocity) {
        // Very light fall, do nothing
        return;
    }

    const auto intensity = FMath::Clamp((impactVelocity - _minStunVelocity) / (_maxStunVelocity - _minStunVelocity), 0.0f, 1.0f);
    const bool bAffectSmall = intensity < 0.5f;
    const bool bAffectLarge = intensity >= 0.5f;

    if (const auto playerController = Cast<APlayerController>(GetController())) {
        playerController->PlayDynamicForceFeedback(intensity, 0.5f, bAffectLarge, bAffectSmall, bAffectLarge, bAffectLarge);
    }

    _serverInitStun(intensity);
}

void ATantrumCharacterBase::RequestSprint() {
    // Can't sprint while stunned
    if (_isStunned) {
        return;
    }

    GetCharacterMovement()->MaxWalkSpeed = _sprintSpeed;
    // You get in here from the client side, because inputs are read by clients.
    // If the replica is sprinting but the authority is not, you'll get pulled back or suffer a stutter effect.
    _serverSprintStart();
}

void ATantrumCharacterBase::RequestSprintCancelation() {
    // Can't sprint while stunned
    if (_isStunned) {
        return;
    }

    GetCharacterMovement()->MaxWalkSpeed = _walkSpeed;
}

bool ATantrumCharacterBase::IsPullingObject() const {
    return _characterThrowState == ECharacterThrowState::RequestingPull || _characterThrowState == ECharacterThrowState::Pulling;
}

bool ATantrumCharacterBase::IsThrowing() const {
    return _characterThrowState == ECharacterThrowState::Throwing;
}

bool ATantrumCharacterBase::IsHovering() const {
    if (const auto tantrumPlayerState = GetPlayerState<ATantrumPlayerState>()) {
        return tantrumPlayerState->GetCurrentState() != EPlayerGameState::Playing;
    }

    return false;
}

void ATantrumCharacterBase::RequestPull() {
    if (!_isStunned && _characterThrowState == ECharacterThrowState::None) {
        _characterThrowState = ECharacterThrowState::RequestingPull;
        _serverRequestPullObject(true);
    }
}

void ATantrumCharacterBase::RequestPullCancelation() {
    // This makes the animation of the pull stop via IsPullingObject()
    if (_characterThrowState == ECharacterThrowState::RequestingPull) {
        _characterThrowState = ECharacterThrowState::None;
        _serverRequestPullObject(false);
    }
}

bool ATantrumCharacterBase::AttemptPullObjectAtLocation(const FVector& inLocation) {
    if (_characterThrowState != ECharacterThrowState::None && _characterThrowState != ECharacterThrowState::RequestingPull) {
        return false;
    }

    if (_isStunned) {
        return false;
    }

    const auto startLoc = GetActorLocation();
    const auto endLoc = inLocation;
    FHitResult hitResult;

    if (GetWorld()) {
        GetWorld()->LineTraceSingleByChannel(hitResult, startLoc, endLoc, ECollisionChannel::ECC_Visibility);
    }

#if ENABLE_DRAW_DEBUG
    if (CVarDisplayTrace->GetBool()) {
        DrawDebugLine(GetWorld(), startLoc, endLoc, hitResult.bBlockingHit ? FColor::Red : FColor::White, false);
    }
#endif

    _characterThrowState = ECharacterThrowState::RequestingPull;
    _processTraceResult(hitResult, false);

    if (_characterThrowState == ECharacterThrowState::Pulling) {
        return true;
    }

    _characterThrowState = ECharacterThrowState::None;
    return false;
}

void ATantrumCharacterBase::RequestUseObject() {
    ApplyEffect_Implementation(_throwable->GetEffectType(), true);
    _throwable->Destroy();
    ResetThrowableObject();
}

void ATantrumCharacterBase::RequestAim() {
    if (CanAim()) {
        _characterThrowState = ECharacterThrowState::Aiming;
        _serverRequestToggleAim(true);
    }
}

void ATantrumCharacterBase::RequestStopAim() {
    if (_characterThrowState == ECharacterThrowState::Aiming) {
        _characterThrowState = ECharacterThrowState::Attached;
        _serverRequestToggleAim(false);
    }
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
        ResetThrowableObject();
    }
}

void ATantrumCharacterBase::NotifyHitByThrowable(const AThrowable* throwable) {
    _serverInitStun(1.0f);
}

void ATantrumCharacterBase::_serverPullObject_Implementation(AThrowable* throwable) {
    if (IsValid(throwable) && !_isStunned && throwable->Pull(this)) {
        _characterThrowState = ECharacterThrowState::Pulling;
        _throwable = throwable;
        _throwable->ToggleHighlight(false);
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

void ATantrumCharacterBase::_clientThrowableAttached_Implementation(AThrowable* throwable) {
    _characterThrowState = ECharacterThrowState::Attached;
    _throwable = throwable;
    MoveIgnoreActorAdd(_throwable.Get());
}

void ATantrumCharacterBase::Tick(const float deltaSeconds) {
    Super::Tick(deltaSeconds);

    if (IsBeingRescued()) {
        _updateRescue(deltaSeconds);
        return;
    }

    /* The replica does not need to concern itself with trying to throw an object and doing raycasts for objects.
     * Consider you have a 4 player game, and let's consider player #2 for example.
     * The character of player 2 is replicated 3 times on machines 1, 3, and 4. However, only the character on machine #2 will be able, through the player
     * controller, to pull and throw objects, because the actual player #2, on their machine, will do so. The characters on machines 1, 3 and 4, being replicas,
     * do not need to execute the logic for pulling and throwing, they just need to show players #1, #3, and #4 that player #2, on their machine, is doing so.
     */
    if (!IsLocallyControlled()) {
        return;
    }

    if (_isStunned) {
        _serverUpdateStun(deltaSeconds);
    }

    if (_bIsUnderEffect) {
        _updateEffect(deltaSeconds);
    }

    if (_isStunned || _bIsUnderEffect) {
        return;
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

void ATantrumCharacterBase::_multicastPlayCelebrateMontage_Implementation() {
    _playCelebrateMontage();
}

void ATantrumCharacterBase::OnRep_CharacterThrowState(const ECharacterThrowState& oldCharacterThrowState) {
    if (_characterThrowState != oldCharacterThrowState) {
        UE_LOG(LogTemp, Warning, TEXT("%s(): OldThrowState: %s"), *FString{__FUNCTION__}, *UEnum::GetDisplayValueAsText(oldCharacterThrowState).ToString());
        UE_LOG(LogTemp, Warning, TEXT("%s(): CharacterThrowState: %s"), *FString{__FUNCTION__}, *UEnum::GetDisplayValueAsText(_characterThrowState).ToString());
    }
}

void ATantrumCharacterBase::OnRep_CharacterIsStunned(const bool oldIsStunned) {
    if (_isStunned != oldIsStunned) {
        const auto oldIsStunnedString = oldIsStunned ? FString{TEXT("True")} : FString{TEXT("False")};
        UE_LOG(LogTemp, Warning, TEXT("%s(): OldIsStunned: %s"), *FString{__FUNCTION__}, *oldIsStunnedString);

        const auto isStunnedString = _isStunned ? FString{TEXT("True")} : FString{TEXT("False")};
        UE_LOG(LogTemp, Warning, TEXT("%s(): NewIsStunned: %s"), *FString{__FUNCTION__}, *isStunnedString);
    }
}

void ATantrumCharacterBase::OnRep_CharacterIsUnderEffect(bool oldIsUnderEffect) {
    if (_bIsUnderEffect != oldIsUnderEffect) {
        const auto oldIsUnderEffectString = oldIsUnderEffect ? FString{TEXT("True")} : FString{TEXT("False")};
        UE_LOG(LogTemp, Warning, TEXT("%s(): OldIsStunned: %s"), *FString{__FUNCTION__}, *oldIsUnderEffectString);

        const auto isUnderEffectString = _bIsUnderEffect ? FString{TEXT("True")} : FString{TEXT("False")};
        UE_LOG(LogTemp, Warning, TEXT("%s(): NewIsStunned: %s"), *FString{__FUNCTION__}, *isUnderEffectString);
    }
}

void ATantrumCharacterBase::OnRep_IsBeingRescued() {
    if (_isBeingRescued) {
        _startRescue();
    } else {
        _endRescue();
    }
}

void ATantrumCharacterBase::_serverInitStun_Implementation(const float stunIntensity) {
    const auto netModeString = _getNetModeDebugString();
    UE_LOG(LogTemp, Warning, TEXT("%s() called in net mode %s"), *FString{__FUNCTION__}, *netModeString);

    if (_isStunned) {
        // For now just early exit. Alternative option would be to add to the stun time
        return;
    }

    _stunDuration = stunIntensity * (_maxStunDuration - _minStunDuration);
    _stunTime = 0.f;
    _isStunned = true;
    GetCharacterMovement()->MaxWalkSpeed = 0.f;

    ResetThrowableObject();
}

void ATantrumCharacterBase::_serverUpdateStun_Implementation(float deltaSeconds) {
    const auto netModeString = _getNetModeDebugString();
    UE_LOG(LogTemp, Warning, TEXT("%s() called in net mode %s"), *FString{__FUNCTION__}, *netModeString);

    check(_isStunned);

    _stunTime += deltaSeconds;
    if (_stunTime > _stunDuration) {
        GetCharacterMovement()->MaxWalkSpeed = _walkSpeed;
        _stunTime = 0.0f;
        _isStunned = false;
        return;
    }

    GetCharacterMovement()->MaxWalkSpeed = _walkSpeed * (_stunTime / _stunDuration);
}

void ATantrumCharacterBase::_clientResetThrowable_Implementation() {
    _characterThrowState = ECharacterThrowState::None;
    _throwable = nullptr;
}

void ATantrumCharacterBase::_startRescue() {
    const auto netModeString = _getNetModeDebugString();
    UE_LOG(LogTemp, Warning, TEXT("%s() called in net mode %s"), *FString{__FUNCTION__}, *netModeString);

    _isBeingRescued = true;
    // _fallOutOfWorldPosition isn't replicated, only _lastGroundPosition is. Smooth transition from arbitrary start point
    // to a common, server-dictated, position.
    _fallOutOfWorldPosition = GetActorLocation();
    _currentRescueTime = 0.0f;
    GetCharacterMovement()->Deactivate();
    SetActorEnableCollision(false);
}

void ATantrumCharacterBase::_updateRescue(const float deltaTime) {
    if (!IsBeingRescued()) {
        const auto netModeString = _getNetModeDebugString();
        UE_LOG(LogTemp, Error, TEXT("%s() called with false _isBeingRescued in net mode %s"), *FString{__FUNCTION__}, *netModeString);
    }

    _currentRescueTime += deltaTime;
    const auto alpha = FMath::Clamp(_currentRescueTime / _timeToRescuePlayer, 0.0f, 1.0f);
    const auto newPlayerLocation = FMath::Lerp(_fallOutOfWorldPosition, _lastGroundPosition, alpha);
    SetActorLocation(newPlayerLocation);

    if (HasAuthority() && alpha >= 1.0f) {
        _endRescue();
    }
}

void ATantrumCharacterBase::_endRescue() {
    const auto netModeString = _getNetModeDebugString();
    UE_LOG(LogTemp, Warning, TEXT("%s() called in net mode %s"), *FString{__FUNCTION__}, *netModeString);

    _isBeingRescued = false;
    GetCharacterMovement()->Activate();
    SetActorEnableCollision(true);
    _currentRescueTime = 0.0f;
}

void ATantrumCharacterBase::ApplyEffect_Implementation(EEffectType effectType, bool bIsBuff) {
    _serverInitEffect(effectType, bIsBuff);
}

void ATantrumCharacterBase::_serverInitEffect_Implementation(const EEffectType effectType, const bool bIsBuff) {
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

void ATantrumCharacterBase::_updateEffect_Implementation(const float deltaTime) {
    if (_effectCooldown > 0.0f) {
        _effectCooldown -= deltaTime;
    } else {
        _effectCooldown = _defaultEffectCooldown;
        _endEffect();
    }
}

void ATantrumCharacterBase::_endEffect_Implementation() {
    _bIsUnderEffect = false;
    switch (_currentEffect) {
        case EEffectType::Speed:
            _bIsEffectBuff ? _sprintSpeed *= 0.5f, RequestSprintCancelation() : GetCharacterMovement()->SetMovementMode(MOVE_Walking);
        default:
            break;
    }
}

void ATantrumCharacterBase::FellOutOfWorld(const UDamageType& dmgType) {
    // DO NOT call the Super, or you'll get a Destroy() and make the entier rescue logic useless
    // Super::FellOutOfWorld(dmgType);

    if (HasAuthority()) {
        _startRescue();
    }
}

void ATantrumCharacterBase::OnMovementModeChanged(EMovementMode prevMovementMode, uint8 previousCustomMode) {
    // Authority takes care of _lastGroundPosition, which is replicated on clients
    if (HasAuthority()) {
        if (!IsBeingRescued() && (prevMovementMode == MOVE_Walking && GetCharacterMovement()->MovementMode == MOVE_Falling)) {
            _lastGroundPosition = GetActorLocation() + GetActorForwardVector() * -100.0f + GetActorUpVector() * 100.0f;
        }
    }
    Super::OnMovementModeChanged(prevMovementMode, previousCustomMode);
}

void ATantrumCharacterBase::BeginPlay() {
    Super::BeginPlay();

    SetReplicateMovement(true); // Character movement replication

    _effectCooldown = _defaultEffectCooldown;
}

bool ATantrumCharacterBase::_playThrowMontage() {
    const float playRate = 1.0f;
    const bool bPlayedSuccessfully = PlayAnimMontage(_throwMontage, playRate) > 0.0f;

    if (bPlayedSuccessfully) {
        // We don't want replicas to bind to these delegates, that are for logic stuff, we only bound the callbacks if we're locally controlled.
        if (IsLocallyControlled()) {
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
    }

    return bPlayedSuccessfully;
}

bool ATantrumCharacterBase::_playCelebrateMontage() {
    const float playRate = 1.0f;
    const bool bPlayedSuccessfully = PlayAnimMontage(_celebrateMontage, playRate) > 0.0f;

    // We don't want replicas to bind to these delegates
    if (bPlayedSuccessfully) {
        // Differently from _playThrowMontage(), we want everyone, replicas included to bind to this delegate.
        // The reason is that we want the extension of the celebration (the one beyond the "Winner" point in the montage to show for each replica.
        const auto animInstance = GetMesh()->GetAnimInstance();

        // Setting the end delegate callback on the montage
        if (!_montageEndedDelegate.IsBound()) {
            _montageEndedDelegate.BindUObject(this, &ATantrumCharacterBase::_onMontageEnded);
        }
        animInstance->Montage_SetEndDelegate(_montageEndedDelegate, _celebrateMontage);
    }

    return bPlayedSuccessfully;
}

void ATantrumCharacterBase::_serverSprintStart_Implementation() {
    GetCharacterMovement()->MaxWalkSpeed = _sprintSpeed;
}

void ATantrumCharacterBase::_serverRequestPullObject_Implementation(bool bIsPulling) {
    _characterThrowState = bIsPulling ? ECharacterThrowState::RequestingPull : ECharacterThrowState::None;
}

void ATantrumCharacterBase::_sphereCastPlayerView() {
    FVector location;
    FRotator rotation;

    const auto controller = GetController();
    check(IsValid(controller)); // This should always be valid since we're getting here only if locally controlled.
    controller->GetPlayerViewPoint(location, rotation);

    const auto playerViewForward = rotation.Vector();
    const auto additionalDistance = (location - GetActorLocation()).Size(); // player to camera distance
    const auto endLocation = location + (playerViewForward * (_pullRange + additionalDistance));

    const auto characterForwardVector = GetActorForwardVector();
    const auto dotProduct = FVector::DotProduct(playerViewForward, characterForwardVector);

    // Prevent picking up objects behind the character. This happens when the pull input is given with the camera looking
    // at the character's front side.
    if (dotProduct < -0.23) {
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
    UKismetSystemLibrary::SphereTraceSingle(GetWorld(), location, endLocation, _pullSphereTraceRadius,
        UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility), false, actorsToIgnore, debugTrace, hitResult, true);
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

    UKismetSystemLibrary::SphereTraceSingle(GetWorld(), startLoc, endLoc, _debugPointRadius,
        UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility), false, TArray<AActor*>{}, debugTrace, hitResult, true);
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

void ATantrumCharacterBase::_processTraceResult(const FHitResult& hitResult, const bool bHighlight /* = true */) {
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

    if (!isValidTarget || _isStunned) {
        return;
    }

    if (!isSameActor) {
        _throwable = hitThrowable;
        if (bHighlight) {
            _throwable->ToggleHighlight(true);
        }
    }

    if (_characterThrowState == ECharacterThrowState::RequestingPull) {
        // You can't pull while sprinting
        if (FMath::IsNearlyEqual(GetCharacterMovement()->MaxWalkSpeed, _sprintSpeed) || GetCharacterMovement()->MaxWalkSpeed > _sprintSpeed) {
            return;
        }

        _serverPullObject(_throwable.Get());
        _characterThrowState = ECharacterThrowState::Pulling;
        _throwable->ToggleHighlight(false);
    }
}

void ATantrumCharacterBase::_onMontageBlendingOut(UAnimMontage* montage, bool bInterrupted) {
}

void ATantrumCharacterBase::_onMontageEnded(UAnimMontage* montage, bool bInterrupted) {
    if (IsLocallyControlled()) {
        _unbindMontage();
    }

    if (montage == _throwMontage) {
        if (IsLocallyControlled()) {
            _characterThrowState = ECharacterThrowState::None;
            _serverFinishThrow();
            _throwable = nullptr;
        }
    } else if (montage == _celebrateMontage) {
        if (const auto tantrumPlayerState = GetPlayerState<ATantrumPlayerState>()) {
            if (tantrumPlayerState->IsWinner()) {
                // Plays the 2nd part of the montage only if you won.
                PlayAnimMontage(_celebrateMontage, 1.0f, TEXT("Winner"));
            }
        }
    }
}

void ATantrumCharacterBase::_unbindMontage() {
    if (!IsLocallyControlled()) {
        return;
    }

    if (const auto animInstance = GetMesh()->GetAnimInstance()) {
        animInstance->OnPlayMontageNotifyBegin.RemoveDynamic(this, &ATantrumCharacterBase::_onNotifyBeginReceived);
        animInstance->OnPlayMontageNotifyEnd.RemoveDynamic(this, &ATantrumCharacterBase::_onNotifyEndReceived);
    }
}

void ATantrumCharacterBase::ResetThrowableObject() {
    if (_throwable.IsValid()) {
        _throwable->Drop();
    }
    _characterThrowState = ECharacterThrowState::None;
    _throwable = nullptr;

    _clientResetThrowable();
}

void ATantrumCharacterBase::OnThrowableAttached(AThrowable* throwable) {
    _characterThrowState = ECharacterThrowState::Attached;
    _throwable = throwable;
    MoveIgnoreActorAdd(_throwable.Get());

    // _onThrowableAttached() gets executed on the server. So we need to tell the client that actually owns this character to execute the logic through this
    // function. This is not a broadcast, because we're telling this to just one of the clients. For example, the _throwable attribute isn't replicated, so we
    // need to tell the client to update it.
    _clientThrowableAttached(throwable);
}

void ATantrumCharacterBase::ServerPlayCelebrateMontage_Implementation() {
    _multicastPlayCelebrateMontage();
}

void ATantrumCharacterBase::_onNotifyBeginReceived(FName notifyName, const FBranchingPointNotifyPayload& branchingPointNotifyPayload) {
    if (notifyName != TEXT("ThrowNotify")) {
        return;
    }

    // The call to AThrowable::Throw() must be performed on the server, it has the ownership of the throwables, on the clients they are just replicas.
    _serverBeginThrow();
}

void ATantrumCharacterBase::_serverBeginThrow_Implementation() {
    // To guard against the fact that you might be forced to drop while throwing. For example, this can happen if you are hit by a throwable while throwing a
    // throwable.
    if (!_throwable.IsValid()) {
        return;
    }

    if (_throwable->GetRootComponent()) {
        if (const auto throwableRoot = Cast<UPrimitiveComponent>(_throwable->GetRootComponent())) {
            throwableRoot->IgnoreActorWhenMoving(this, true);
        }
    }

    const auto throwVelocity = GetActorForwardVector() * _throwSpeed;
    _throwable->Throw(throwVelocity);

    if (CVarDisplayThrowVelocity->GetBool()) {
        const auto start = GetMesh()->GetSocketLocation(TEXT("ObjectAttach"));
        DrawDebugLine(GetWorld(), start, start + throwVelocity, FColor::Red, false, 5.0f);
    }

    const auto start = GetMesh()->GetSocketLocation(TEXT("ObjectAttach"));
    UE_VLOG_ARROW(this, LogTantrumChar, Verbose, start, start + throwVelocity, FColor::Red, TEXT("Throw Direction"));
}

void ATantrumCharacterBase::_serverFinishThrow_Implementation() {
    _characterThrowState = ECharacterThrowState::None;

    // To guard against the fact that you might be forced to drop while throwing. For example, this can happen if you are hit by a throwable while throwing a
    // throwable.
    if (!_throwable.IsValid()) {
        return;
    }

    MoveIgnoreActorRemove(_throwable.Get());

    if (_throwable->GetRootComponent()) {
        if (const auto throwableRoot = Cast<UPrimitiveComponent>(_throwable->GetRootComponent())) {
            throwableRoot->IgnoreActorWhenMoving(this, false);
        }
    }

    // Uncomment this function call if you want the throwable to affect the character that threw it too
    // ResetThrowableObject();
    _throwable = nullptr;
}

void ATantrumCharacterBase::_onNotifyEndReceived(FName notifyName, const FBranchingPointNotifyPayload& branchingPointNotifyPayload) {
}

FString ATantrumCharacterBase::_getNetModeDebugString() const {
    const auto netMode = GetNetMode();
    auto netModeString = FString{};
    switch (netMode) {
        case NM_Client:
            netModeString = "client";
            break;
        case NM_ListenServer:
            netModeString = "listen server";
            break;
        default:
            netModeString = "NONE";
    }

    return netModeString;
}

void ATantrumCharacterBase::_serverRequestToggleAim_Implementation(bool isAiming) {
    _characterThrowState = isAiming ? ECharacterThrowState::Aiming : ECharacterThrowState::Attached;
}
