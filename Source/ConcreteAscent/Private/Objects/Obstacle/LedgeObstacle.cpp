// Fill out your copyright notice in the Description page of Project Settings.


#include "Objects/Obstacle/LedgeObstacle.h"
#include "Components/BoxComponent.h"

bool ALedgeObstacle::GetLedgeMoveSegment(const FVector& FrontNormal, FVector& OutCenter, FVector& OutRight, float& OutHalfLength) const
{
	OutCenter = FVector::ZeroVector;
	OutRight = FVector::RightVector;
	OutHalfLength = 0.f;

	if (!TraversalBounds)
		return false;

	const FTransform BoundsTransform = TraversalBounds->GetComponentTransform();
	const FVector ScaledExtent = TraversalBounds->GetScaledBoxExtent();

	FVector LocalFrontNormal =
		BoundsTransform.InverseTransformVectorNoScale(FrontNormal);

	LocalFrontNormal.Z = 0.f;

	if (!LocalFrontNormal.Normalize())
		return false;

	const float AbsX = FMath::Abs(LocalFrontNormal.X);
	const float AbsY = FMath::Abs(LocalFrontNormal.Y);

	FVector CenterLocal = FVector::ZeroVector;
	FVector RightLocal = FVector::RightVector;
	float BoundsHalfLength = 0.f;

	if (AbsX >= AbsY)
	{
		const float SignX = LocalFrontNormal.X >= 0.f ? 1.f : -1.f;

		CenterLocal = FVector(SignX * TraversalBounds->GetUnscaledBoxExtent().X, 0.f, TraversalBounds->GetUnscaledBoxExtent().Z);

		RightLocal = FVector(0.f, 1.f, 0.f);
		BoundsHalfLength = ScaledExtent.Y;
	}
	else
	{
		const float SignY = LocalFrontNormal.Y >= 0.f ? 1.f : -1.f;

		CenterLocal = FVector(0.f, SignY * TraversalBounds->GetUnscaledBoxExtent().Y, TraversalBounds->GetUnscaledBoxExtent().Z);

		RightLocal = FVector(1.f, 0.f, 0.f);
		BoundsHalfLength = ScaledExtent.X;
	}

	OutCenter = BoundsTransform.TransformPosition(CenterLocal);
	OutRight = BoundsTransform.TransformVectorNoScale(RightLocal).GetSafeNormal();

	if (OutRight.IsNearlyZero())
		return false;

	OutHalfLength = BoundsHalfLength;

	return OutHalfLength > KINDA_SMALL_NUMBER;
}
