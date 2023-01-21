// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Throwable.h"

#include "ThrowAbilityComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TANTRUM_API UThrowAbilityComponent : public UActorComponent {
	GENERATED_BODY()

public:	
	UThrowAbilityComponent();

	bool CanThrow() const;

	void RequestPull();
	void RequestPullCancelation();

	void RequestThrow();

protected:
	void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Throw Ability")
	float _throwSpeed = 1000.0f;

private:
	TWeakObjectPtr<AThrowable> _throwable;
};
