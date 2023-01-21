// Fill out your copyright notice in the Description page of Project Settings.


#include "Throwable.h"

AThrowable::AThrowable() {
	PrimaryActorTick.bCanEverTick = true;

	_staticMeshC = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static Mesh"));
	SetRootComponent(_staticMeshC);

	_projectileMovementC = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement"));

	// This is to avoid the UpdatedComponent of the _projectileMovementC to auto-manage itself (auto-activation and deactivation).
	// Needed beacause otherwise the projectle will turn itself off after laying on the ground for a bit.
	_projectileMovementC->bAutoActivate = false;
}

void AThrowable::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
}


void AThrowable::BeginPlay() {
	Super::BeginPlay();
}
