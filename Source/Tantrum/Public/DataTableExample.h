// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Engine/DataTable.h"
#include "GameFramework/Actor.h"

#include "DataTableExample.generated.h"

/* Mimicking what we have in the InteractInterface.h just to teach how Data Tables work. */
UENUM(BlueprintType)
enum class EDTEffectType : uint8 {
	None  UMETA(DisplayName = "None"),
	Speed UMETA(DisplayName = "SpeedBuff"),
	Jump  UMETA(DisplayName = "JumpBuff"),
	Power UMETA(DisplayName = "PowerBuff")
};


/* Inheriting from FTableRowBase allows us to use this struct as row for our data table. */
USTRUCT(BlueprintType)
struct FEffectStats : public FTableRowBase {
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EDTEffectType EffectType = EDTEffectType::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float EffectStrenght = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString EffectDescription = "";
};

UCLASS()
class TANTRUM_API ADataTableExample : public AActor {
	GENERATED_BODY()
	
public:	
	ADataTableExample();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data")
	TObjectPtr<UDataTable> EffectsTable;

	// To easily search through in BP
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data")
	EDTEffectType SelectedEffectBP = EDTEffectType::None;

	// To easily search through in C++
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data")
	FName SelectedEffect = "";

	// Variables we can check in the Editor against the information stored in the Data Table
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data")
	EDTEffectType EffectType = EDTEffectType::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data")
	float EffectStrenght = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data")
	FString EffectDescription = "";

protected:
	void BeginPlay() override;
};
