// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Animation/AnimMontage.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
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
class TANTRUM_API ATantrumCharacterBase : public ACharacter {
	GENERATED_BODY()

public:
	ATantrumCharacterBase();

	UFUNCTION(BlueprintPure)
	bool IsPullingObject() const;

	void RequestPull();
	void RequestPullCancelation();

	bool CanThrow() const;
	void RequestThrow();

	void Tick(float deltaSeconds) override;

protected:
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, Category = Camera)
	TObjectPtr<USpringArmComponent> _cameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, Category = Camera)
	TObjectPtr<UCameraComponent> _followCamera;

private:
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
};
