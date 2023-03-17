// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/StaticMeshComponent.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "InteractInterface.h"

#include "Throwable.generated.h"

UCLASS()
class TANTRUM_API AThrowable : public AActor {
    GENERATED_BODY()

public:
    AThrowable();

    bool Pull(TWeakObjectPtr<ACharacter> pullCharacter);

    void Drop();

    void Throw(const FVector& throwDirection);

    UFUNCTION(BlueprintPure)
    bool IsIdle() const { return _state == EThrowState::Idle; }

    /**
     * \brief Attaches this component to other actor if other is the _pullCharacter
     */
    void NotifyHit(UPrimitiveComponent* myComp, AActor* other, UPrimitiveComponent* otherComp, bool bSelfMoved, FVector hitLocation, FVector hitNormal,
        FVector normalImpulse, const FHitResult& hit) override;

    void ToggleHighlight(bool bIsOn);

    EEffectType GetEffectType() const { return _effectType; }

protected:
    void BeginPlay() override;

    void EndPlay(EEndPlayReason::Type endPlayReason) override;

    UPROPERTY(EditAnywhere, Category = "Throwable")
    float _initialZVelocity = 1000.0f;

private:
    bool _setHomingTarget(TWeakObjectPtr<AActor> target);

    UFUNCTION()
    void _projectileStop(const FHitResult& impactResult);

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> _staticMeshC;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UProjectileMovementComponent> _projectileMovementC;

    // The actor that will pull this throwable
    TWeakObjectPtr<ACharacter> _pullCharacter = nullptr;

    enum class EThrowState : uint8
    {
        Idle,
        Pull,
        Attached,
        Throw,
        Dropped,
    };

    EThrowState _state = EThrowState::Idle;

    UPROPERTY(EditAnywhere, Category = "Effect")
    EEffectType _effectType = EEffectType::None;
};
