// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_RequestSprint.h"

#include "AIController.h"
#include "TantrumCharacterBase.h"

UBTTask_RequestSprint::UBTTask_RequestSprint() {
    NodeName = TEXT("Request Sprint");
}

EBTNodeResult::Type UBTTask_RequestSprint::ExecuteTask(UBehaviorTreeComponent& ownerComp, uint8* nodeMemory) {
    Super::ExecuteTask(ownerComp, nodeMemory);

    const auto aiController = ownerComp.GetAIOwner();
    if (const auto tantrumCharacter = Cast<ATantrumCharacterBase>(aiController->GetPawn()); IsValid(tantrumCharacter)) {
        tantrumCharacter->RequestSprint();
        return EBTNodeResult::Succeeded;
    }

    UE_LOG(LogTemp, Error, TEXT("%s(): AI Owner does not control a TantrumCharacterBase Character"), *FString{__FUNCTION__});
    return EBTNodeResult::Failed;
}
