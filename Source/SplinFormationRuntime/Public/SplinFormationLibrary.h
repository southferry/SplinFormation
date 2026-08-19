
#pragma once

#include "CoreMinimal.h"
#include "LandscapeSplineActor.h"
#include "SplinFormationLibrary.generated.h"


UENUM(BlueprintType)
enum class EHouseType : uint8
{
    Regular UMETA(DisplayName = "Regular"),
    End     UMETA(DisplayName = "End")
};

USTRUCT(BlueprintType)
struct FHouseOption
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Houses")
    FString Key = "Default House Key";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Houses")
    float Width = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Houses")
    float BreezeWidth = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Houses")
    EHouseType Type = EHouseType::Regular;
};

USTRUCT(BlueprintType)
struct FLayoutResult
{
    GENERATED_BODY()

    // The actual selected houses in order
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Houses")
    TArray<FHouseOption> Sequence;
};


UCLASS()
class SPLINFORMATIONRUNTIME_API USplinFormationLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    UFUNCTION(BlueprintCallable, Category = "Splinformation")
    static void CopyTerrainSpline(ALandscapeSplineActor* LSA, USplineComponent* Destination);

    UFUNCTION(BlueprintCallable, Category = "Splinformation")
    static void GenerateOffsetSpline(USplineComponent* Base, USplineComponent* Target, float LateralOffset = 0.f, float ZOffset = 0.f, float CloneDensity = 100.f, bool reverse = false);

    UFUNCTION(BlueprintCallable, Category = "Houses")
    static bool RandomPackHouses(const TArray<FHouseOption>& Houses, float OverallWidth, bool Breezeway, bool EndHouses, FLayoutResult& OutLayout);

};
