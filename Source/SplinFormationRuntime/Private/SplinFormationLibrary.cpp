
#include "SplinFormationLibrary.h"
#include "LandscapeSplineActor.h"
#include "LandscapeSplinesComponent.h"

#include "Algo/Reverse.h"

#include "LandscapeSplineSegment.h"
#include "LandscapeSplineControlPoint.h"
#include "Misc/DateTime.h"

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

		FRotator FirstRot = Base->GetRotationAtDistanceAlongSpline(0.0f, ESplineCoordinateSpace::World);
		double StartingYaw = reverse ? EndRotation.Yaw : FirstRot.Yaw;
		
		Target->SetWorldRotation(FRotator(0, StartingYaw, 0));
		
		Target->UpdateSpline();
		Target->ClearSplinePoints(false);

		Target->SetSplinePoints(Points, ESplineCoordinateSpace::World, false);
		Target->UpdateSpline();
	}
}


/**
	HOUSE GEN PACKING HELPERS
**/

void ShuffleHouses(TArray<FHouseOption>& Arr)
{
	int32 Num = Arr.Num();
	if (Num <= 1) return;

	for (int32 i = 0; i < Num - 1; ++i)
	{
		int32 j = FMath::RandRange(i, Num - 1);
		if (i != j)
		{
			Arr.Swap(i, j);
		}
	}
}

int32 widthOfHouses(bool Breezeway, bool StartsEven, int32 HWidth, int32 BWidth, int32 Num)
{
	int32 TotalWidth = 0;
	for (int32 i = 0; i < Num; i++)
	{
		bool Even = (i % 2 == 0);
		int32 CurrWidth = HWidth;
		if (Breezeway && ((StartsEven && Even) || (!StartsEven && !Even)))
			CurrWidth += BWidth;
		TotalWidth += CurrWidth;
	}
	return TotalWidth;
}

int32 numHousesForWidth(bool Breezeway, bool StartsEven, int32 HWidth, int32 BWidth, int32 TargetWidth)
{
	int32 Count = 0;
	while (true)
	{
		int32 Width = widthOfHouses(Breezeway, StartsEven, HWidth, BWidth, Count);
		if (Width > TargetWidth)
			return Count - 1;
		Count++;
	}
}


bool RandomPacking(
	const TArray<FHouseOption>& Houses,
	int32 OverallWidth,
	bool Breezeway,
	FLayoutResult& OutLayout)
{
	OutLayout.Sequence.Empty();

	TArray<const FHouseOption*> Candidates;
	for (auto& H : Houses)
	{
		int32 AsInt = FMath::RoundToInt(H.Width);
		if (!FMath::IsNearlyEqual(H.Width, (float)AsInt, 1e-3f))
		{
			UE_LOG(LogTemp, Warning, TEXT("One of the House Widths was not close enough to an integer"));
			return false;
		}

		int32 BrAsInt = FMath::RoundToInt(H.BreezeWidth);
		if (!FMath::IsNearlyEqual(H.BreezeWidth, (float)BrAsInt, 1e-3f))
		{
			UE_LOG(LogTemp, Warning, TEXT("One of the House Breezeway Widths was not close enough to an integer"));
			return false;
		}

		Candidates.Add(&H);
	}

	// --- Generate all start/end pairings ---
	struct FState { TArray<FHouseOption> Seq; };
	TArray<FLayoutResult> ValidResults;

	const int32 Remaining = OverallWidth;

	FState Initial;
	Initial.Seq = { };

	// Recursive lambda for *middle* section
	TFunction<void(int32, int32, FState&)> Search =
		[&](int32 Index, int32 RemainingWidth, FState& State)
		{
			constexpr float EPS = 0.01f;

			// termination
			if (RemainingWidth <= EPS)
			{
				// close sequence by appending end house
				FState Completed = State;
				ShuffleHouses(Completed.Seq);

				FLayoutResult Result;
				Result.Sequence = Completed.Seq;

				ValidResults.Add(MoveTemp(Result));
				return;
			}

			//evaluated last house, not a solution
			if (Index >= Candidates.Num())
				return;

			int32 HouseIndex = State.Seq.Num();
			bool EvenIndex = (HouseIndex % 2 == 0);

			
			const FHouseOption* Candidate = Candidates[Index];
			const int32 Width = FMath::RoundToInt(Candidate->Width);
			const int32 BrWidth = FMath::RoundToInt(Candidate->BreezeWidth);
			const int32 MaxCount = numHousesForWidth(Breezeway, EvenIndex, Width, BrWidth, RemainingWidth);

			for (int32 Count = 0; Count <= MaxCount; ++Count)
			{
				FState Next = State;

				for (int32 i = 0; i < Count; ++i)
				{
					Next.Seq.Add(*Candidate);
				}

				Search(Index + 1, RemainingWidth - widthOfHouses(Breezeway, EvenIndex, Width, BrWidth, Count), Next);
			}
		};

	Search(0, Remaining, Initial);
	

	if (ValidResults.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No valid combinations found."));
		return false;
	}

	const int32 Picked = FMath::RandHelper(ValidResults.Num());
	OutLayout = ValidResults[Picked];
	return true;
}

bool RandomTypedPacking(
	const TArray<FHouseOption>& Houses,
	int32 OverallWidth,
	bool Breezeway,
	FLayoutResult& OutLayout)
{
	OutLayout.Sequence.Empty();

	// --- Identify start/end eligible sets ---
	TArray<const FHouseOption*> StartCandidates;
	TArray<const FHouseOption*> EndCandidates;
	TArray<const FHouseOption*> MiddleCandidates;

	for (auto& H : Houses)
	{
		int32 AsInt = FMath::RoundToInt(H.Width);
		if (!FMath::IsNearlyEqual(H.Width, (float)AsInt, 1e-3f))
		{
			UE_LOG(LogTemp, Warning, TEXT("One of the House Widths was not close enough to an integer"));
			return false;
		}
		
		if (H.Type == EHouseType::End)
		{
			StartCandidates.Add(&H);
			EndCandidates.Add(&H);
		}
		else {
			MiddleCandidates.Add(&H);
		}
	}

	if (StartCandidates.Num() == 0 || EndCandidates.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No eligible start or end houses."));
		return false;
	}

	// --- Generate all start/end pairings ---
	struct FState { TArray<FHouseOption> Seq; };
	TArray<FLayoutResult> ValidResults;

	for (const FHouseOption* Start : StartCandidates)
	{
		for (const FHouseOption* End : EndCandidates)
		{
			const float StartWidth = Breezeway ? Start->Width + Start->BreezeWidth : Start->Width;
			const int32 Remaining = OverallWidth - (FMath::RoundToInt(StartWidth) + FMath::RoundToInt(End->Width));
			if (Remaining < 0)
				continue;

			FState Initial;
			Initial.Seq = { };
			//Initial.Counts.FindOrAdd(Start->Type)++;

			// Recursive lambda for *middle* section
			TFunction<void(int32, int32, FState&)> Search =
				[&](int32 Index, int32 RemainingWidth, FState& State)
				{
					constexpr float EPS = 0.01f;

					// termination
					if (RemainingWidth <= EPS)
					{
						// close sequence by appending end house
						FState Completed = State;
						ShuffleHouses(Completed.Seq);

						FLayoutResult Result;
						Result.Sequence = {*Start};
						Result.Sequence.Append(Completed.Seq);
						Result.Sequence.Add(*End);

						ValidResults.Add(MoveTemp(Result));
						return;
					}

					//evaluated last house, not a solution
					if (Index >= MiddleCandidates.Num())
						return;

					// Determine if starting position even/odd (offset by 1 b/c start house)
					int32 HouseIndex = State.Seq.Num() + 1;
					bool EvenIndex = (HouseIndex % 2 == 0);

					const FHouseOption* Candidate = MiddleCandidates[Index];
					const int32 Width = FMath::RoundToInt(Candidate->Width);
					const int32 BrWidth = FMath::RoundToInt(Candidate->BreezeWidth);
					// How many of this house, factoring breezeways, fully fit in the space remaining
					const int32 MaxCount = numHousesForWidth(Breezeway, EvenIndex, Width, BrWidth, RemainingWidth);

					for (int32 Count = 0; Count <= MaxCount; ++Count)
					{
						FState Next = State;

						for (int32 i = 0; i < Count; ++i)
						{
							Next.Seq.Add(*Candidate);
						}

						Search(Index + 1, RemainingWidth - widthOfHouses(Breezeway, EvenIndex, Width, BrWidth, Count), Next);
					}
				};

			Search(0, Remaining, Initial);
		}
	}

	if (ValidResults.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No valid start/middle/end combinations found."));
		return false;
	}

	const int32 Picked = FMath::RandHelper(ValidResults.Num());
	OutLayout = ValidResults[Picked];
	return true;
}

bool USplinFormationLibrary::RandomPackHouses(const TArray<FHouseOption>& Houses, float OverallWidth, bool Breezeway, bool EndHouses, FLayoutResult& OutLayout)
{

	if (Houses.Num() == 0 || OverallWidth <= 0.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid input. No houses or non-positive width."));
		return false;
	}

	const int32 IntOverall = FMath::RoundToInt(OverallWidth);
	if (!FMath::IsNearlyEqual(OverallWidth, (float)IntOverall, 1e-3f))
	{
		UE_LOG(LogTemp, Warning, TEXT("The target width was not close enough to an integer"));
		return false;
	}

	if (EndHouses) 
	{
		return RandomTypedPacking(Houses, IntOverall, Breezeway, OutLayout);
	}
	else {
		return RandomPacking(Houses, IntOverall, Breezeway, OutLayout);
	}
}