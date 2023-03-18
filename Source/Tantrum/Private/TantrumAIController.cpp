// Fill out your copyright notice in the Description page of Project Settings.

#include "TantrumAIController.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TantrumCharacterBase.h"
#include "TantrumGameStateBase.h"
#include "TantrumPlayerState.h"

ATantrumAIController::ATantrumAIController() {
    // Needed for GetPlayerState() to work related logic from the AI Controller
    bWantsPlayerState = true;
}

void ATantrumAIController::OnPossess(APawn* const inPawn) {
    Super::OnPossess(inPawn);

    if (const auto tantrumCharacter = Cast<ATantrumCharacterBase>(inPawn)) {
        if (const auto tantrumPlayerState = GetPlayerState<ATantrumPlayerState>()) {
            tantrumPlayerState->SetCurrentState(EPlayerGameState::Waiting);
        }
    }
}

void ATantrumAIController::OnUnPossess() {
    Super::OnUnPossess();
}

void ATantrumAIController::Tick(float deltaTime) {
    Super::Tick(deltaTime);

    const auto tantrumGameState = Cast<ATantrumGameStateBase>(GetWorld()->GetGameState());
    if (IsValid(tantrumGameState)) {
        const auto blackBoard = GetBlackboardComponent();
        blackBoard->SetValueAsBool(_isPlayingKeyName, tantrumGameState->IsPlaying());
    }
}

void ATantrumAIController::OnReachedEnd() {
    if (const auto tantrumCharacter = Cast<ATantrumCharacterBase>(GetCharacter())) {
        tantrumCharacter->ServerPlayCelebrateMontage();
    }
}

void ATantrumAIController::BeginPlay() {
    Super::BeginPlay();

    if (!IsValid(_behaviorTree)) {
        UE_LOG(LogTemp, Error, TEXT("%s(): Invalid Behavior Tree!"), *FString{__FUNCTION__});
        return;
    }

    RunBehaviorTree(_behaviorTree);

    if (!IsValid(_goalActorClass)) {
        UE_LOG(LogTemp, Error, TEXT("%s(): Invalid Goal Actor Class!"), *FString{__FUNCTION__});
        return;
    }

    TArray<AActor*> actorsOfClass;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), _goalActorClass, actorsOfClass);

    if (!actorsOfClass.IsEmpty()) {
        const auto goalActor = actorsOfClass.Last();

        const auto blackBoard = GetBlackboardComponent();
        blackBoard->SetValueAsVector(_destinationKeyName, goalActor->GetActorLocation());
    }
}
