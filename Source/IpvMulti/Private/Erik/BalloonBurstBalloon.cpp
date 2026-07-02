// Copyright © 2026 IpvMulti - Erik

#include "Erik/BalloonBurstBalloon.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

ABalloonBurstBalloon::ABalloonBurstBalloon()
{
	PrimaryActorTick.bCanEverTick = true;

	BalloonMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BalloonMesh"));
	RootComponent = BalloonMesh;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshAsset(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMeshAsset.Succeeded())
	{
		BalloonMesh->SetStaticMesh(SphereMeshAsset.Object);
	}

	BalloonMesh->SetCollisionProfileName(TEXT("BlockAll"));
	BalloonMesh->SetGenerateOverlapEvents(false);
}

void ABalloonBurstBalloon::BeginPlay()
{
	Super::BeginPlay();
	ResetBalloon();
}

void ABalloonBurstBalloon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsInflating || bHasBurst)
	{
		return;
	}

	const float NewScale = CurrentScale + (InflationRate * DeltaTime);
	if (NewScale >= MaxScale)
	{
		ApplyScale(MaxScale);
		Burst();
		return;
	}

	ApplyScale(NewScale);
}

void ABalloonBurstBalloon::StartInflating()
{
	if (bHasBurst)
	{
		return;
	}

	bIsInflating = true;
}

void ABalloonBurstBalloon::StopInflating()
{
	bIsInflating = false;
}

void ABalloonBurstBalloon::ResetBalloon()
{
	bIsInflating = false;
	bHasBurst = false;
	ApplyScale(MinScale);

	if (BalloonMesh)
	{
		BalloonMesh->SetVisibility(true);
		BalloonMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
}

void ABalloonBurstBalloon::ApplyScale(const float NewScale)
{
	CurrentScale = NewScale;
	SetActorScale3D(FVector(CurrentScale));
}

void ABalloonBurstBalloon::Burst()
{
	if (bHasBurst)
	{
		return;
	}

	bHasBurst = true;
	bIsInflating = false;

	OnBurstEffects();
	OnBalloonBurst.Broadcast();

	if (BalloonMesh)
	{
		BalloonMesh->SetVisibility(false);
		BalloonMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (bDestroyOnBurst)
	{
		Destroy();
	}
}
