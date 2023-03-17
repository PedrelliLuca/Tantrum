// Fill out your copyright notice in the Description page of Project Settings.

#include "DataTableExample.h"

ADataTableExample::ADataTableExample() {
    PrimaryActorTick.bCanEverTick = false;
}

void ADataTableExample::BeginPlay() {
    Super::BeginPlay();

    if (EffectsTable) {
        const FString contextString{TEXT("Effect Context")};
        const auto effectStats = EffectsTable->FindRow<FEffectStats>(SelectedEffect, contextString, true);

        EffectType = effectStats->EffectType;
        EffectStrenght = effectStats->EffectStrenght;
        EffectDescription = effectStats->EffectDescription;
    }
}
