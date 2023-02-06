// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Animation/AnimMontage.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "InteractInterface.h"
#include "Throwable.h"

#include "TantrumCharacterBase.generated.h"

UENUM(BlueprintType)
enum class ECharacterThrowState : uint8 {
	None           UMETA(DisplayName = "None"),
	RequestingPull UMETA(DisplayName = "Requesting Pull"),
	Pulling        UMETA(DisplayName = "Pulling"),
	Attached       UMETA(DisplayName = "Attached"),
	Throwing       UMETA(DisplayName = "Throwing"),
};

UCLASS()
class TANTRUM_API ATantrumCharacterBase : public ACharacter, public IInteractInterface {
	GENERATED_BODY()

public:
	ATantrumCharacterBase();

	void Landed(const FHitResult& hit) override;

	void RequestSprint();
	void RequestSprintCancelation();

	UFUNCTION(BlueprintPure)
	bool IsPullingObject() const;

	void RequestPull();
	void RequestPullCancelation();

	// To use the object rather than throwing it.
	void RequestUseObject();

	bool CanThrow() const;
	void RequestThrow();

	void Tick(float deltaSeconds) override;

	void ApplyEffect_Implementation(EEffectType effectType, bool bIsBuff) override;

	void EndEffect();

protected:
	void BeginPlay() override;

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, Category = Camera)
	TObjectPtr<USpringArmComponent> _cameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, Category = Camera)
	TObjectPtr<UCameraComponent> _followCamera;

private:
	void _updateStun(float deltaSeconds);
	bool _isStunned() const { return _stunTime > 0.0f; }

	bool _playThrowMontage();

	void _sphereCastPlayerView();
	void _sphereCastActorTransform();
	void _lineCastActorTransform();

	void _processTraceResult(const FHitResult& hitResult);

	void _onMontageBlendingOut(UAnimMontage* montage, bool bInterrupted);
	void _onMontageEnded(UAnimMontage* montage, bool bInterrupted);
	void _unbindMontage();

	void _setThrowable(TWeakObjectPtr<AThrowable> newThrowable);

	void _resetThrowableObject();
	void _onThrowableAttached();

	UFUNCTION()
	void _onNotifyBeginReceived(FName notifyName, const FBranchingPointNotifyPayload& branchingPointNotifyPayload);
	UFUNCTION()
	void _onNotifyEndReceived(FName notifyName, const FBranchingPointNotifyPayload& branchingPointNotifyPayload);

	UPROPERTY(EditAnywhere, Category = "Input")
	float _sprintSpeed = 1200.0f;

	UPROPERTY(EditAnywhere, Category = "Input")
	float _walkSpeed = 600.0f;

	// The final velocity for falling from 3m
	UPROPERTY(EditAnywhere, Category = "Stun")
	float _minStunVelocity = 800.f;

	UPROPERTY(EditAnywhere, Category = "Stun")
	float _maxStunVelocity = 1600.f;

	UPROPERTY(EditAnywhere, Category = "Stun")
	float _maxStunDuration = 10.f;

	UPROPERTY(EditAnywhere, Category = "Stun")
	float _minStunDuration = 1.f;

	float _stunDuration = 0.f;
	float _stunTime = -1.f;

	UPROPERTY(VisibleAnywhere, Category = "Throw")
	ECharacterThrowState _characterThrowState = ECharacterThrowState::None;

	TWeakObjectPtr<AThrowable> _throwable = nullptr;

	UPROPERTY(EditAnywhere, Category = "Throw", meta = (ClampMin = "0.0"))
	float _pullRange = 1000.0f;

	UPROPERTY(EditAnywhere, Category = "Throw", meta = (ClampMin = "0.0"))
	float _pullSphereTraceRadius = 70.0f;

	UPROPERTY(EditAnywhere, Category = "Throw", meta = (ClampMin = "0.0"))
	float _debugPointRadius = 70.0f;

	UPROPERTY(EditAnywhere, Category = "Throw", meta = (ClampMin = "0.0"))
	float _throwSpeed = 2000.0f;

	UPROPERTY(EditAnywhere, Category = "Animation")
	TObjectPtr<UAnimMontage> _throwMontage = nullptr;

	FOnMontageBlendingOutStarted _blendingOutDelegate;
	FOnMontageEnded _montageEndedDelegate;

	bool _bIsUnderEffect = false;
	bool _bIsEffectBuff = false;

	UPROPERTY(EditAnywhere, Category = "Effects", meta = (ClampMin = "0.0"))
	float _defaultEffectCooldown = 5.0f;
	float _effectCooldown = 0.0f;

	EEffectType _currentEffect = EEffectType::None;
};
