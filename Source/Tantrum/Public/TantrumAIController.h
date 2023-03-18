// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Perception/AIPerceptionComponent.h"
#include "Throwable.h"

#include "TantrumAIController.generated.h"

/**
 *
 */
UCLASS()
class TANTRUM_API ATantrumAIController : public AAIController {
    GENERATED_BODY()

public:
    ATantrumAIController();

    void OnPossess(APawn* inPawn) override;

    void OnUnPossess() override;

    void Tick(float deltaTime) override;

    // Mimicks ATantrumPlayerController::ClientReachedEnd() by playing the celebrate montage but without showing any UI
    void OnReachedEnd();

protected:
    void BeginPlay() override;

private:
    UFUNCTION()
    void _onActorSensed(AActor* actor, FAIStimulus stimulus);

    void _setThrowableAsDestination();

    void _setRaceEndAsDestination();

    UPROPERTY(EditDefaultsOnly, Category = "Tantrum AI | Behavior Tree")
    TObjectPtr<UBehaviorTree> _behaviorTree;

    UPROPERTY(EditDefaultsOnly, Category = "Tantrum AI | Goal Class")
    TSubclassOf<AActor> _goalActorClass;

    UPROPERTY(EditDefaultsOnly, Category = "Tantrum AI | Goal Class")
    TSubclassOf<AThrowable> _throwableActorClass;

    UPROPERTY(EditDefaultsOnly, Category = "Tantrum AI | Blackboard Keys")
    FName _destinationKeyName = "Destination";

    UPROPERTY(EditDefaultsOnly, Category = "Tantrum AI | Blackboard Keys")
    FName _isPlayingKeyName = "IsPlaying";

    UPROPERTY(EditDefaultsOnly, Category = "Tantrum AI | Blackboard Keys")
    FName _canThrowKeyName = "CanThrow";

    UPROPERTY(EditDefaultsOnly, Category = "Tantrum AI | Blackboard Keys")
    FName _targetEnemyKeyName = "TargetEnemy";

    UPROPERTY(VisibleAnywhere, Category = "Chaser | Perception")
    TObjectPtr<UAIPerceptionComponent> _perceptionC;
};
