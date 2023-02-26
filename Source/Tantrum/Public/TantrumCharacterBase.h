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

	// Tells the server which attributes need to be replicated on the replicas.
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& outLifetimeProps) const override;

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

	void ResetThrowableObject();
	void OnThrowableAttached(AThrowable* throwable);

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

	UFUNCTION(Server, Reliable)
	void _serverPullObject(AThrowable* throwable);
	// The reason we don't have a _multicastRequestPullObject() is that we move to the Interact state of the "Actions" anim FSM based on the "Is Interacting" boolean,
	// whose value is set based on IsPullingObject(). This latter uses the _characterThrowState, which is already replicated.
	UFUNCTION(Server, Reliable)
	void _serverRequestPullObject(bool bIsPulling);

	// RPC for playing the anim montage when throwing objects
	UFUNCTION(Server, Reliable)
	void _serverRequestThrowObject();
	UFUNCTION(NetMulticast, Reliable)
	void _multicastRequestThrowObject();

	// Client-only function
	UFUNCTION(Client, Reliable)
	void _clientThrowableAttached(AThrowable* throwable);

	UFUNCTION(Server, Reliable)
	void _serverBeginThrow();

	UFUNCTION(Server, Reliable)
	void _serverFinishThrow();

	UFUNCTION()
	void OnRep_CharacterThrowState(const ECharacterThrowState& oldCharacterThrowState);


	void _sphereCastPlayerView();
	void _sphereCastActorTransform();
	void _lineCastActorTransform();

	void _processTraceResult(const FHitResult& hitResult);

	void _onMontageBlendingOut(UAnimMontage* montage, bool bInterrupted);
	void _onMontageEnded(UAnimMontage* montage, bool bInterrupted);
	void _unbindMontage();

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

	/* Replicated using an OnRep function, which means "when this value changes, call the OnRep_CharacterThrowState function". Where is this function called?
	 * OnRep_XXX() IS ONLY CALLED ON THE SERVER, only the server replica will know this. The client cannot communicate this value to the replicas on other clients directly. 
	 * To tell the server to replicate the change on the replicas, you need to override GetLifetimeReplicatedProps(), which is what we're doing for this class. */
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_CharacterThrowState, Category = "Throw")
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
