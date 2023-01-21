// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "GameFramework/ProjectileMovementComponent.h"

#include "Throwable.generated.h"

UCLASS()
class TANTRUM_API AThrowable : public AActor {
	GENERATED_BODY()
	
public:	
	AThrowable();

	bool Pull(TWeakObjectPtr<ACharacter> pullCharacter);

	void Throw(const FVector& throwDirection);

	/**
	* \brief Attaches this component to other actor if other is the _pullCharacter
	*/
	void NotifyHit(UPrimitiveComponent* myComp, AActor* other, UPrimitiveComponent* otherComp, bool bSelfMoved, FVector hitLocation, FVector hitNormal, FVector normalImpulse, const FHitResult& hit) override;

protected:
	void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Throwable")
	float _initialZVelocity = 1000.0f;

private:
	bool _setHomingTarget(TWeakObjectPtr<AActor> target);

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> _staticMeshC;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UProjectileMovementComponent> _projectileMovementC;

	// The actor that will pull this throwable
	TWeakObjectPtr<ACharacter> _pullCharacter = nullptr;
};
