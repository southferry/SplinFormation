// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LandscapeSplineActor.h"
#include "SplinFormationLibrary.generated.h"

UCLASS()
class SPLINFORMATIONRUNTIME_API USplinFormationLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    UFUNCTION(BlueprintCallable, Category = "Splinformation")
    static void CopyTerrainSpline(ALandscapeSplineActor* LSA, USplineComponent* Destination);

    UFUNCTION(BlueprintCallable, Category = "Splinformation")
    static void GenerateOffsetSpline(USplineComponent* Base, USplineComponent* Target, float LateralOffset = 0.f, float ZOffset = 0.f, float CloneDensity = 100.f, bool reverse = false);

    UFUNCTION(BlueprintCallable, Category = "House|Random")
    static bool RandomPacking(const TArray<float>& Widths, float OverallWidth, TArray<int32>& OutCounts);

};
