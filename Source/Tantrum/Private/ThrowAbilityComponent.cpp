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

void UThrowAbilityComponent::BeginPlay() {
	Super::BeginPlay();
	check(IsValid(Cast<ACharacter>(GetOwner())));
}
