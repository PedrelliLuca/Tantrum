// Fill out your copyright notice in the Description page of Project Settings.

#include "AISpawner.h"

AAISpawner::AAISpawner() {
    PrimaryActorTick.bCanEverTick = false;
}

void AAISpawner::SpawnAI() const {
    FActorSpawnParameters spawnParams;
    spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    const auto actor = GetWorld()->SpawnActor(_actorClassToSpawn, &GetActorTransform(), spawnParams);
    if (!IsValid(actor)) {
        UE_LOG(LogTemp, Error, TEXT("%s(): Spawn failed!"), *FString{__FUNCTION__});
    }
}
