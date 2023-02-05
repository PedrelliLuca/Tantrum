// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractInterface.generated.h"

UINTERFACE(MinimalAPI)
class UInteractInterface : public UInterface {
	GENERATED_BODY()
};

UENUM(BlueprintType)
enum class EEffectType : uint8 {
	None  UMETA(DisplayName = "None"),
	Speed UMETA(DisplayName = "SpeedBuff"),
	Jump  UMETA(DisplayName = "JumpBuff"),
	Power UMETA(DisplayName = "PowerBuff")
};

/**
 * 
 */
class TANTRUM_API IInteractInterface {
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interact")
	void ApplyEffect(EEffectType effectType, bool bIsBuff);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interact")
	EEffectType UseEffect();
};
