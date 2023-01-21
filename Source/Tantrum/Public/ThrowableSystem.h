// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Throwable.h"

#include "ThrowableSystem.generated.h"

/**
 * 
 */
UCLASS()
class TANTRUM_API UThrowableSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	void AddThrowable(TWeakObjectPtr<AThrowable> throwable);

	TWeakObjectPtr<AThrowable> GetThrowableClosestTo(const FVector& location);

private:
	UFUNCTION()
	void _throwableDestroyed(AActor* destroyedThrowableAsActor);

	TSet<TWeakObjectPtr<AThrowable>> _throwables;
};
