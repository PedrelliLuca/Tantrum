// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "CoreMinimal.h"
#include "BTTask_AttemptPullObject.generated.h"

/**
 *
 */
UCLASS()
class TANTRUM_API UBTTask_AttemptPullObject : public UBTTask_BlackboardBase {
    GENERATED_BODY()

public:
    UBTTask_AttemptPullObject();

private:
    EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& ownerComp, uint8* nodeMemory) override;
};
