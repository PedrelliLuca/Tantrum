// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AISpawner.generated.h"

UCLASS()
class TANTRUM_API AAISpawner : public AActor {
    GENERATED_BODY()

public:
    AAISpawner();

    // Called from the level BP
    UFUNCTION(BlueprintCallable)
    void SpawnAI() const;

private:
    UPROPERTY(EditAnywhere, Category = "AI Spawner")
    TSubclassOf<AActor> _actorClassToSpawn = nullptr;
};
