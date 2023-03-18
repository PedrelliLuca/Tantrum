// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_AttemptPullObject.h"

#include "AIController.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "TantrumCharacterBase.h"

UBTTask_AttemptPullObject::UBTTask_AttemptPullObject() {
    NodeName = "Attempt Pull Object";
    bNotifyTick = false;
    bNotifyTaskFinished = true;

    BlackboardKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_AttemptPullObject, BlackboardKey));
}

EBTNodeResult::Type UBTTask_AttemptPullObject::ExecuteTask(UBehaviorTreeComponent& ownerComp, uint8* nodeMemory) {
    Super::ExecuteTask(ownerComp, nodeMemory);

    const auto aiController = ownerComp.GetAIOwner();
    if (const auto tantrumCharacter = Cast<ATantrumCharacterBase>(aiController->GetPawn()); IsValid(tantrumCharacter)) {
        const auto blackboard = ownerComp.GetBlackboardComponent();
        check(IsValid(blackboard));
        const auto targetLocation = blackboard->GetValue<UBlackboardKeyType_Vector>(BlackboardKey.GetSelectedKeyID());

        return tantrumCharacter->AttemptPullObjectAtLocation(targetLocation) ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
    }

    UE_LOG(LogTemp, Error, TEXT("%s(): AI Owner does not control a TantrumCharacterBase Character"), *FString{__FUNCTION__});
    return EBTNodeResult::Failed;
}
