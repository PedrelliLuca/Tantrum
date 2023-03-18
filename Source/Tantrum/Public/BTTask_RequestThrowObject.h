// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BehaviorTree/BTTaskNode.h"
#include "CoreMinimal.h"
#include "BTTask_RequestThrowObject.generated.h"

/**
 *
 */
UCLASS()
class TANTRUM_API UBTTask_RequestThrowObject : public UBTTaskNode {
    GENERATED_BODY()

public:
    UBTTask_RequestThrowObject();

private:
    EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& ownerComp, uint8* nodeMemory) override;
};
