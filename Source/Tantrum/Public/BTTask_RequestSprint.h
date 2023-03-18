// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BehaviorTree/BTTaskNode.h"
#include "CoreMinimal.h"
#include "BTTask_RequestSprint.generated.h"

/**
 *
 */
UCLASS()
class TANTRUM_API UBTTask_RequestSprint : public UBTTaskNode {
    GENERATED_BODY()

public:
    UBTTask_RequestSprint();

private:
    EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& ownerComp, uint8* nodeMemory) override;
};
