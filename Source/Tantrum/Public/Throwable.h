// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/ProjectileMovementComponent.h"

#include "Throwable.generated.h"

UCLASS()
class TANTRUM_API AThrowable : public AActor
{
	GENERATED_BODY()
	
public:	
	AThrowable();

	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> _staticMeshC;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UProjectileMovementComponent> _projectileMovementC;
};
