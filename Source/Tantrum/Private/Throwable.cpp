// Fill out your copyright notice in the Description page of Project Settings.


#include "Throwable.h"

#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "ThrowableSystem.h"

AThrowable::AThrowable() {
	PrimaryActorTick.bCanEverTick = false;

	_staticMeshC = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static Mesh"));
	SetRootComponent(_staticMeshC);

	_projectileMovementC = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement"));

	// This is to avoid the UpdatedComponent of the _projectileMovementC to auto-manage itself (auto-activation and deactivation).
	// Needed beacause otherwise the projectle will turn itself off after laying on the ground for a bit.
	_projectileMovementC->bAutoActivate = false;
	_projectileMovementC->bShouldBounce = true;
	_projectileMovementC->HomingAccelerationMagnitude = 1000.0f;
}

bool AThrowable::Pull(TWeakObjectPtr<ACharacter> pullCharacter) {
	check(pullCharacter.IsValid());

	/*if (_state != EThrowState::Idle) {
		return false;
	}*/

	if (_setHomingTarget(pullCharacter)) {
		// ToggleHighlight(false);
		// _state = EThrowState::Pull;
		_pullCharacter = MoveTemp(pullCharacter);
		return true;
	}

	return false;
}

void AThrowable::Throw(const FVector& throwDirection) {
	// if (_state == EThrowState::Pull || _state == EThrowState::Attached) { ... all the code below }

	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	_projectileMovementC->Activate(true);
	_projectileMovementC->HomingTargetComponent = nullptr;

	// _state = EThrowState::Throw;

	// TODO: add AActor* target input that can be optionally set. If set, its root component is chosen as new HomingTargetComponent

	_projectileMovementC->Velocity = throwDirection + FVector::UpVector * _initialZVelocity;
}

void AThrowable::NotifyHit(UPrimitiveComponent* myComp, AActor* other, UPrimitiveComponent* otherComp, bool bSelfMoved, FVector hitLocation, FVector hitNormal, FVector normalImpulse, const FHitResult& hit) {
	Super::NotifyHit(myComp, other, otherComp, bSelfMoved, hitLocation, hitNormal, normalImpulse, hit);
	/*if (_state == EThrowState::Idle || _state == EThrowState::Attached || _state == EThrowState::Dropped) {
		return;
	}*/

	if (!_pullCharacter.IsValid()) {
		UE_LOG(LogTemp, Warning, TEXT("%s: Invalid _pullCharacter"), __FUNCTION__);
		return;
	}

	// if (_state == EThrowState::Pull) { ... if-else below ... }

	if (other == _pullCharacter) {
		AttachToComponent(_pullCharacter->GetCapsuleComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("Throwable Attach"));
		SetOwner(_pullCharacter.Get());
		_projectileMovementC->Deactivate();
		// _state = EThrowState::Attached;
		// character->OnThrowableAttached();
	} else {
		// character->ResetThrowableObject();
		// _state = EThrowState::Dropped;
	}
	
	_projectileMovementC->HomingTargetComponent = nullptr;
	_pullCharacter = nullptr;
}

void AThrowable::BeginPlay() {
	Super::BeginPlay();

	const auto throwSystem = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UThrowableSystem>();
	throwSystem->AddThrowable(this);
}

bool AThrowable::_setHomingTarget(TWeakObjectPtr<AActor> target) {
	check(target.IsValid());

	if (const auto sceneC = target->FindComponentByClass<USceneComponent>()) {
		_projectileMovementC->SetUpdatedComponent(_staticMeshC);
		_projectileMovementC->Activate(true);
		_projectileMovementC->HomingTargetComponent = sceneC;
		_projectileMovementC->Velocity = FVector::UpVector * _initialZVelocity;
		return true;
	}

	return false;
}
