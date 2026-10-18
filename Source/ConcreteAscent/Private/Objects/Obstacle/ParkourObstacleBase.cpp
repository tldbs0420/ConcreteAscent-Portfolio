// Fill out your copyright notice in the Description page of Project Settings.


#include "Objects/Obstacle/ParkourObstacleBase.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"

AParkourObstacleBase::AParkourObstacleBase()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	// ObstacleMesh는 시각 표현만 담당하고, 실제 파쿠르 판정은 TraversalBounds가 담당한다.
	ObstacleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ObstacleMesh"));
	ObstacleMesh->SetupAttachment(SceneRoot);
	ObstacleMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ObstacleMesh->SetGenerateOverlapEvents(false);

	// TraversalBounds는 장애물의 탐지, 높이, 두께, Ledge 위치 계산 기준으로 사용한다.
	TraversalBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("TraversalBounds"));
	TraversalBounds->SetupAttachment(SceneRoot);
	TraversalBounds->SetBoxExtent(FVector(15.f, 50.f, 50.f));
	TraversalBounds->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	TraversalBounds->SetCollisionObjectType(ECC_WorldStatic);
	TraversalBounds->SetCollisionResponseToAllChannels(ECR_Block);
	TraversalBounds->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
}

void AParkourObstacleBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (!TraversalBounds)
		return;

	// 에디터에서 TraversalBounds 크기가 바뀌었을 때 확인용 높이와 두께 값을 갱신한다.
	const FVector ScaledExtent = TraversalBounds->GetScaledBoxExtent();

	Height = ScaledExtent.Z * 2.f;
	Thickness = FMath::Min(ScaledExtent.X, ScaledExtent.Y) * 2.f;
}

bool AParkourObstacleBase::GetTraversalLedgeData_Implementation(
	const FHitResult& HitResult,
	const FVector& ActorLocation,
	FVector& OutFrontLedgeLocation,
	FVector& OutFrontLedgeNormal,
	FVector& OutBackLedgeLocation,
	FVector& OutBackLedgeNormal,
	float& OutObstacleHeight,
	float& OutObstacleDepth
) const
{
	if (!TraversalBounds)
		return false;

	const FTransform BoundsTransform = TraversalBounds->GetComponentTransform();
	const FVector BoxExtent = TraversalBounds->GetUnscaledBoxExtent();

	// 플레이어 위치는 장애물의 어느 면에서 접근했는지 판단하는 기준으로 사용한다.
	const FVector LocalActorLocation = BoundsTransform.InverseTransformPosition(ActorLocation);

	// 충돌 지점은 Ledge의 좌우 위치를 결정하는 기준으로 사용한다.
	const FVector ReferenceWorldLocation = HitResult.bBlockingHit ? HitResult.ImpactPoint : ActorLocation;
	const FVector LocalReferenceLocation = BoundsTransform.InverseTransformPosition(ReferenceWorldLocation);

	// BoxExtent가 0에 가까울 때 나눗셈 오류가 발생하지 않도록 보정한다.
	const float SafeExtentX = FMath::Max(BoxExtent.X, KINDA_SMALL_NUMBER);
	const float SafeExtentY = FMath::Max(BoxExtent.Y, KINDA_SMALL_NUMBER);

	// 로컬 좌표 기준으로 X축 면과 Y축 면 중 어느 쪽에서 더 가깝게 접근했는지 판단한다.
	const float NormalizedX = FMath::Abs(LocalActorLocation.X) / SafeExtentX;
	const float NormalizedY = FMath::Abs(LocalActorLocation.Y) / SafeExtentY;

	FVector FrontNormalLocal = FVector::ZeroVector;
	FVector BackNormalLocal = FVector::ZeroVector;

	FVector FrontLedgeLocal = FVector::ZeroVector;
	FVector BackLedgeLocal = FVector::ZeroVector;

	if (NormalizedX >= NormalizedY)
	{
		const float SignX = LocalActorLocation.X >= 0.f ? 1.f : -1.f;
		const float ClampedY = FMath::Clamp(LocalReferenceLocation.Y, -BoxExtent.Y, BoxExtent.Y);

		// 플레이어가 접근한 X축 방향 면을 FrontLedge로 사용한다.
		FrontNormalLocal = FVector(SignX, 0.f, 0.f);
		BackNormalLocal = -FrontNormalLocal;

		FrontLedgeLocal = FVector(SignX * BoxExtent.X, ClampedY, BoxExtent.Z);
		BackLedgeLocal = FVector(-SignX * BoxExtent.X, ClampedY, BoxExtent.Z);
	}
	else
	{
		const float SignY = LocalActorLocation.Y >= 0.f ? 1.f : -1.f;
		const float ClampedX = FMath::Clamp(LocalReferenceLocation.X, -BoxExtent.X, BoxExtent.X);

		// 플레이어가 접근한 Y축 방향 면을 FrontLedge로 사용한다.
		FrontNormalLocal = FVector(0.f, SignY, 0.f);
		BackNormalLocal = -FrontNormalLocal;

		FrontLedgeLocal = FVector(ClampedX, SignY * BoxExtent.Y, BoxExtent.Z);
		BackLedgeLocal = FVector(ClampedX, -SignY * BoxExtent.Y, BoxExtent.Z);
	}

	// 계산된 로컬 Ledge 위치를 월드 좌표로 변환한다.
	OutFrontLedgeLocation = BoundsTransform.TransformPosition(FrontLedgeLocal);
	OutBackLedgeLocation = BoundsTransform.TransformPosition(BackLedgeLocal);

	// 로컬 Normal을 월드 방향으로 변환한다. 크기는 필요 없으므로 Scale은 제외한다.
	OutFrontLedgeNormal = BoundsTransform.TransformVectorNoScale(FrontNormalLocal).GetSafeNormal();
	OutBackLedgeNormal = BoundsTransform.TransformVectorNoScale(BackNormalLocal).GetSafeNormal();

	// 장애물 자체의 높이와 깊이를 보조 정보로 반환한다.
	OutObstacleHeight = TraversalBounds->GetScaledBoxExtent().Z * 2.f;
	OutObstacleDepth = FVector::Dist2D(OutFrontLedgeLocation, OutBackLedgeLocation);

	return true;
}

