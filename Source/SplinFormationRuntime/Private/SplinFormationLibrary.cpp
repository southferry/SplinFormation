// Fill out your copyright notice in the Description page of Project Settings.


#include "SplinFormationLibrary.h"
#include "LandscapeSplineActor.h"
#include "LandscapeSplinesComponent.h"

#include "Algo/Reverse.h"

#include "LandscapeSplineSegment.h"
#include "LandscapeSplineControlPoint.h"

#if WITH_EDITOR
void USplinFormationLibrary::CopyTerrainSpline(ALandscapeSplineActor* LSA, USplineComponent* Destination)
{
	if (!LSA)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid LandscapeSplineActor!"));
		return;
	}
	ULandscapeSplinesComponent* LSC = LSA->GetSplinesComponent();
	if (!LSC)
	{
		UE_LOG(LogTemp, Warning, TEXT("Error getting spline from LandscapeSplineActor!"));
		return;
	}

	//Copy from Landscape into temp to avoid pitfalls of landscape 
	USplineComponent* TempSpline = NewObject<USplineComponent>(GetTransientPackage(), USplineComponent::StaticClass());

	LSC->CopyToSplineComponent(TempSpline);
	
	const TArray<TObjectPtr<ULandscapeSplineControlPoint>> Points = LSC->GetControlPoints();
	if (Points.Num() > 0)
	{
		int32 PNum = 0;
		TObjectPtr<ULandscapeSplineControlPoint> LastPoint = Points.Last();

		FVector WorldLoc = LSA->GetActorLocation() + LastPoint->Location;
		TempSpline->AddSplinePoint(WorldLoc, ESplineCoordinateSpace::World, true);
		TempSpline->UpdateSpline();
	}


	Destination->ClearSplinePoints();
	int32 TempPointCount = TempSpline->GetNumberOfSplinePoints();
	for (int32 i = 0; i < TempPointCount; i++)
	{
		FSplinePoint TempPoint = TempSpline->GetSplinePointAt(i, ESplineCoordinateSpace::World);
		if (i == 0)
			Destination->SetWorldLocation(TempPoint.Position);
		Destination->AddSplinePoint(TempPoint.Position, ESplineCoordinateSpace::World, true);
	}

	//This looks ridiculous but we DO have to go through it again to add proper tangents with everything connected
	//Also skip the first and the last because the tangents cause weird behaviors on those nodes
	for (int32 i = 1; i < TempPointCount - 1; i++)
	{
		FSplinePoint TempPoint = TempSpline->GetSplinePointAt(i, ESplineCoordinateSpace::World);
		Destination->SetTangentsAtSplinePoint(i, TempPoint.ArriveTangent, TempPoint.LeaveTangent, ESplineCoordinateSpace::World, true);
	}
	Destination->Modify(); // Marks it dirty for transaction
	Destination->MarkPackageDirty();
}
#endif



void USplinFormationLibrary::GenerateOffsetSpline(USplineComponent* Base, USplineComponent* Target, float LateralOffset, float ZOffset, float CloneDensity, bool reverse)
{
	//reset target
	
	
	const float SplineLength = Base->GetSplineLength();
	UE_LOG(LogTemp, Warning, TEXT("Base Length: %f"), SplineLength);

	const int32 NumPoints = FMath::CeilToInt(SplineLength / CloneDensity);
	UE_LOG(LogTemp, Warning, TEXT("Num Points: %d"), NumPoints);

	const FVector ZVector = FVector(0.f, 0.f, ZOffset);

	TArray<FVector> Points;
	FRotator EndRotation;
	for (int32 i = 0; i <= NumPoints; i++)
	{
		float IncrementDistance = i * CloneDensity;
		float Distance = (Base->GetSplineLength() > IncrementDistance) ? IncrementDistance : Base->GetSplineLength();
		if (i == NumPoints)
			EndRotation = Base->GetRotationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World) + FRotator(0.f, 180.f, 0.f);
		FVector Location = Base->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
		FVector RightVec = Base->GetRightVectorAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
		//FVector N_Tan = Base->GetTangentAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World).GetSafeNormal();

		FVector PointLocation = Location + (RightVec * LateralOffset) + ZVector;
		Points.Add(PointLocation);

	}	

	UE_LOG(LogTemp, Warning, TEXT("Actual Generated Points: %d"), Points.Num());

	if (Points.Num() > 0)
	{
		if (reverse)
		{
			Algo::Reverse(Points);
		}



		Target->SetWorldLocation(Points[0]);
		if (reverse)
		{
			Target->SetWorldRotation(EndRotation);
		} else {
			Target->SetWorldRotation(Base->GetWorldRotationAtDistanceAlongSpline(0.f));
		}
		Target->UpdateSpline();
		Target->ClearSplinePoints(false);

		Target->SetSplinePoints(Points, ESplineCoordinateSpace::World, false);
		Target->UpdateSpline();
	}
}

bool USplinFormationLibrary::RandomPacking(const TArray<float>& Widths, float OverallWidth, TArray<int32>& OutCounts)
{
	

	OutCounts.Empty();

	// Validate Inputs
	const int32 N = Widths.Num();
	if (N == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Inputs were invalid (House 'Widths' is empty, likely House Array is Empty)"));
		return false;
	}
	if (OverallWidth <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Inputs were invalid ('OverallWidth' <= 0)"));
		return false;
	}

	TArray<int32> IntWidths;
	IntWidths.Reserve(Widths.Num());

	for (float W : Widths)
	{
		int32 AsInt = FMath::RoundToInt(W);
		if (!FMath::IsNearlyEqual(W, (float)AsInt, 1e-3f))
		{
			UE_LOG(LogTemp, Warning, TEXT("One of the House Widths was not close enough to an integer"));
			return false;
		}
		IntWidths.Add(AsInt);
	}

	int32 IntOverall = FMath::RoundToInt(OverallWidth);
	UE_LOG(LogTemp, Warning, TEXT("IntOverall: %d"), IntOverall);
	if (!FMath::IsNearlyEqual(OverallWidth, (float)IntOverall, 0.01f))
	{
		UE_LOG(LogTemp, Warning, TEXT("The Overall Width to fill was not close enough to an Integer"));
		return false;
	}


	// Storage for all solutions
	TArray<TArray<int32>> Solutions;

	// Simple recursive lambda to enumerate
	TFunction<void(int32, int32, TArray<int32>&)> Search =
		[&](int32 Index, int32 Remaining, TArray<int32>& Current)
		{
			if (Index == N - 1)
			{
				// Last width: must divide remaining exactly
				if (Remaining % IntWidths[Index] == 0)
				{
					Current[Index] = Remaining / IntWidths[Index];
					Solutions.Add(Current);
				}
				return;
			}

			int32 MaxCount = Remaining / IntWidths[Index];
			for (int32 Count = 0; Count <= MaxCount; Count++)
			{
				Current[Index] = Count;
				Search(Index + 1, Remaining - Count * IntWidths[Index], Current);
			}
		};

	// Kick off recursion
	TArray<int32> Current;
	Current.SetNumZeroed(N);
	Search(0, IntOverall, Current);

	// If no solutions, bail
	if (Solutions.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Not Mathematically Solveable - There are no combinations of these widths that equal the overall width"));
		return false;
	}

	// Pick one at random
	int32 Picked = FMath::RandHelper(Solutions.Num());
	OutCounts = Solutions[Picked];

	return true;
}