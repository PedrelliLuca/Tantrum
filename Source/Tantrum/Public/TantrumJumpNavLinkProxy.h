// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Navigation/NavLinkProxy.h"

#include "TantrumJumpNavLinkProxy.generated.h"

/**
 *
 */
UCLASS()
class TANTRUM_API ATantrumJumpNavLinkProxy : public ANavLinkProxy {
    GENERATED_BODY()

protected:
    void BeginPlay() override;

private:
    UFUNCTION()
    void _onSmartLinkReached(AActor* movingActor, const FVector& destinationPoint);
};
