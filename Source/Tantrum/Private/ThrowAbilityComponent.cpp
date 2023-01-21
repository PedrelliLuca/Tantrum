// Fill out your copyright notice in the Description page of Project Settings.


#include "ThrowAbilityComponent.h"

#include "Kismet/GameplayStatics.h"
#include "ThrowableSystem.h"

UThrowAbilityComponent::UThrowAbilityComponent() {
	PrimaryComponentTick.bCanEverTick = false;
}

bool UThrowAbilityComponent::CanThrow() const {
	return true;
}

void UThrowAbilityComponent::RequestPull() {
	// You can't pull while sprinting or while having a throwable ready to be thrown
	if (GetOwner()->GetVelocity().Size() >= 10.0f) {
		return;
	}

	/*const auto throwSystem = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UThrowableSystem>();
	const auto throwable = throwSystem->GetThrowableClosestTo(GetOwner()->GetActorLocation());*/

	if (_throwable.IsValid() && _throwable->Pull(Cast<ACharacter>(GetOwner()))) {
		_throwable = nullptr;
	}
}

void UThrowAbilityComponent::RequestPullCancelation() {
}

void UThrowAbilityComponent::RequestThrow() {
	if (!_throwable.IsValid()) {
		return;
	}

	const auto throwableRoot = Cast<UPrimitiveComponent>(_throwable->GetRootComponent());
	check(IsValid(throwableRoot));
	throwableRoot->IgnoreActorWhenMoving(GetOwner(), true);

	const auto throwDirection = GetOwner()->GetActorForwardVector() * _throwSpeed;
	_throwable->Throw(throwDirection);
}

void UThrowAbilityComponent::BeginPlay() {
	Super::BeginPlay();
	check(IsValid(Cast<ACharacter>(GetOwner())));
}
