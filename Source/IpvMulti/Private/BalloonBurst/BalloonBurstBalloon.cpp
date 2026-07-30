// Copyright © 2026 IpvMulti

#include "BalloonBurst/BalloonBurstBalloon.h"

#include "BalloonBurst/BalloonBurstPlayerState.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

ABalloonBurstBalloon::ABalloonBurstBalloon()
{
	bReplicates = true;
	SetReplicateMovement(true);
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	BalloonMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BalloonMesh"));
	BalloonMesh->SetupAttachment(Root);
	BalloonMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BalloonMesh->SetIsReplicated(true);

	StringMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StringMesh"));
	StringMesh->SetupAttachment(Root);
	StringMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	StringMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -60.0f));
	StringMesh->SetRelativeScale3D(FVector(0.05f, 0.05f, 1.2f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh.Succeeded())
	{
		BalloonMesh->SetStaticMesh(SphereMesh.Object);
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (CylinderMesh.Succeeded())
	{
		StringMesh->SetStaticMesh(CylinderMesh.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> BasicMat(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (BasicMat.Succeeded())
	{
		BalloonMesh->SetMaterial(0, BasicMat.Object);
		StringMesh->SetMaterial(0, BasicMat.Object);
	}
}

void ABalloonBurstBalloon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABalloonBurstBalloon, CurrentPumps);
	DOREPLIFETIME(ABalloonBurstBalloon, PumpsRequired);
	DOREPLIFETIME(ABalloonBurstBalloon, bHasBurst);
	DOREPLIFETIME(ABalloonBurstBalloon, BalloonColor);
	DOREPLIFETIME(ABalloonBurstBalloon, OwnerPlayerState);
}

void ABalloonBurstBalloon::BeginPlay()
{
	Super::BeginPlay();
	ApplyBalloonColor();
	ApplyVisualScale();
}

void ABalloonBurstBalloon::InitializeBalloon(ABalloonBurstPlayerState* InOwnerState, int32 /*InStationIndex*/, int32 InPumpsRequired, FLinearColor InColor)
{
	if (!HasAuthority())
	{
		return;
	}

	OwnerPlayerState = InOwnerState;
	PumpsRequired = FMath::Max(1, InPumpsRequired);
	BalloonColor = InColor;
	CurrentPumps = 0;
	bHasBurst = false;

	ApplyBalloonColor();
	ApplyVisualScale();
	OnRep_BalloonColor();
}

bool ABalloonBurstBalloon::ServerAddPump()
{
	if (!HasAuthority() || bHasBurst)
	{
		return false;
	}

	CurrentPumps = FMath::Min(CurrentPumps + 1, PumpsRequired);
	OnRep_CurrentPumps();

	if (ABalloonBurstPlayerState* PS = OwnerPlayerState)
	{
		PS->SetPumpCount(CurrentPumps);
	}

	if (CurrentPumps >= PumpsRequired)
	{
		bHasBurst = true;
		OnRep_HasBurst();

		if (ABalloonBurstPlayerState* PS = OwnerPlayerState)
		{
			PS->SetHasBurstBalloon(true);
		}

		return true;
	}

	return false;
}

void ABalloonBurstBalloon::OnRep_CurrentPumps()
{
	ApplyVisualScale();
}

void ABalloonBurstBalloon::OnRep_HasBurst()
{
	if (bHasBurst)
	{
		PlayBurstVisuals();
	}
}

void ABalloonBurstBalloon::OnRep_BalloonColor()
{
	ApplyBalloonColor();
}

void ABalloonBurstBalloon::ApplyVisualScale() const
{
	if (!BalloonMesh)
	{
		return;
	}

	const float Alpha = PumpsRequired > 0
		? static_cast<float>(CurrentPumps) / static_cast<float>(PumpsRequired)
		: 0.0f;
	const float Scale = FMath::Lerp(MinScale, MaxScale, FMath::Clamp(Alpha, 0.0f, 1.0f));
	BalloonMesh->SetRelativeScale3D(FVector(Scale));
}

void ABalloonBurstBalloon::ApplyBalloonColor()
{
	if (!BalloonMesh)
	{
		return;
	}

	if (!BalloonMID)
	{
		BalloonMID = BalloonMesh->CreateAndSetMaterialInstanceDynamic(0);
	}

	if (BalloonMID)
	{
		BalloonMID->SetVectorParameterValue(TEXT("Color"), BalloonColor);
		// Some engine materials use different parameter names.
		BalloonMID->SetVectorParameterValue(TEXT("BaseColor"), BalloonColor);
	}

	if (StringMesh)
	{
		if (UMaterialInstanceDynamic* StringMID = StringMesh->CreateAndSetMaterialInstanceDynamic(0))
		{
			StringMID->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.15f, 0.15f, 0.15f));
			StringMID->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.15f, 0.15f, 0.15f));
		}
	}
}

void ABalloonBurstBalloon::PlayBurstVisuals()
{
	if (BalloonMesh)
	{
		BalloonMesh->SetRelativeScale3D(FVector(BurstScale));
		BalloonMesh->SetVisibility(false, true);
	}

	if (StringMesh)
	{
		StringMesh->SetVisibility(false, true);
	}
}
