// Copyright © 2026 IpvMulti

#include "FallingFloor/FallingFloorTile.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "FallingFloor/FallingFloorGameState.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

AFallingFloorTile::AFallingFloorTile()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	bReplicates = true;
	SetReplicateMovement(true);

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(SceneRoot);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComp->SetCollisionObjectType(ECC_WorldStatic);
	MeshComp->SetCollisionResponseToAllChannels(ECR_Block);
	MeshComp->SetGenerateOverlapEvents(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (CylinderMesh.Succeeded())
	{
		MeshComp->SetStaticMesh(CylinderMesh.Object);
		MeshComp->SetRelativeScale3D(FVector(1.8f, 1.8f, 0.25f));
	}

	// Tall sensor so the standing capsule always overlaps while on the tile.
	OverlapComp = CreateDefaultSubobject<UBoxComponent>(TEXT("OverlapComp"));
	OverlapComp->SetupAttachment(SceneRoot);
	OverlapComp->SetBoxExtent(FVector(100.0f, 100.0f, 80.0f));
	OverlapComp->SetRelativeLocation(FVector(0.0f, 0.0f, 80.0f));
	OverlapComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	OverlapComp->SetCollisionObjectType(ECC_WorldDynamic);
	OverlapComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	OverlapComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	OverlapComp->SetGenerateOverlapEvents(true);
	OverlapComp->OnComponentBeginOverlap.AddDynamic(this, &AFallingFloorTile::OnOverlapBegin);
}

void AFallingFloorTile::BeginPlay()
{
	Super::BeginPlay();

	if (AFallingFloorGameState* GS = GetWorld() ? GetWorld()->GetGameState<AFallingFloorGameState>() : nullptr)
	{
		GS->OnMatchPhaseChanged.AddUniqueDynamic(this, &AFallingFloorTile::HandleMatchPhaseChanged);
		if (GS->IsMatchInProgress())
		{
			HandleMatchPhaseChanged(EFallingFloorMatchPhase::InProgress);
		}
	}
}

void AFallingFloorTile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFallingFloorTile, bHasTriggered);
	DOREPLIFETIME(AFallingFloorTile, bIsFalling);
}

void AFallingFloorTile::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bIsFalling && HasAuthority())
	{
		AddActorWorldOffset(FVector(0.0f, 0.0f, -FallSpeed * DeltaSeconds), false);
	}
}

void AFallingFloorTile::HandleMatchPhaseChanged(EFallingFloorMatchPhase NewPhase)
{
	if (!HasAuthority() || NewPhase != EFallingFloorMatchPhase::InProgress || bHasTriggered)
	{
		return;
	}

	CheckForStandingPlayers();

	// Keep polling briefly: BeginOverlap can be missed if the pawn was already inside.
	GetWorldTimerManager().SetTimer(
		OccupancyPollTimerHandle,
		this,
		&AFallingFloorTile::CheckForStandingPlayers,
		0.2f,
		true);
}

void AFallingFloorTile::CheckForStandingPlayers()
{
	if (!CanTriggerNow() || !OverlapComp)
	{
		return;
	}

	TArray<AActor*> OverlappingActors;
	OverlapComp->GetOverlappingActors(OverlappingActors, bCharactersOnly ? ACharacter::StaticClass() : APawn::StaticClass());
	for (AActor* Actor : OverlappingActors)
	{
		if (IsValidTriggerActor(Actor))
		{
			TriggerTile();
			GetWorldTimerManager().ClearTimer(OccupancyPollTimerHandle);
			return;
		}
	}
}

bool AFallingFloorTile::IsValidTriggerActor(AActor* OtherActor) const
{
	if (!OtherActor)
	{
		return false;
	}

	if (bCharactersOnly)
	{
		return Cast<ACharacter>(OtherActor) != nullptr;
	}

	return Cast<APawn>(OtherActor) != nullptr;
}

bool AFallingFloorTile::CanTriggerNow() const
{
	if (!HasAuthority() || bHasTriggered)
	{
		return false;
	}

	const AFallingFloorGameState* GS = GetWorld() ? GetWorld()->GetGameState<AFallingFloorGameState>() : nullptr;
	return GS && GS->IsMatchInProgress();
}

void AFallingFloorTile::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsValidTriggerActor(OtherActor) || !CanTriggerNow())
	{
		return;
	}

	TriggerTile();
	GetWorldTimerManager().ClearTimer(OccupancyPollTimerHandle);
}

void AFallingFloorTile::TriggerTile()
{
	if (!CanTriggerNow())
	{
		return;
	}

	bHasTriggered = true;
	OnRep_HasTriggered();

	GetWorldTimerManager().ClearTimer(OccupancyPollTimerHandle);
	GetWorldTimerManager().SetTimer(
		FallDelayTimerHandle,
		this,
		&AFallingFloorTile::BeginFall,
		FallDelay,
		false);
}

void AFallingFloorTile::BeginFall()
{
	if (!HasAuthority() || bIsFalling)
	{
		return;
	}

	bIsFalling = true;
	OnRep_IsFalling();
	SetActorTickEnabled(true);
	SetLifeSpan(DestroyAfterFall);
}

void AFallingFloorTile::DisableCollision()
{
	if (OverlapComp)
	{
		OverlapComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		OverlapComp->SetGenerateOverlapEvents(false);
	}

	if (MeshComp)
	{
		MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AFallingFloorTile::OnRep_HasTriggered()
{
	OnTileTriggered();
}

void AFallingFloorTile::OnRep_IsFalling()
{
	DisableCollision();
	OnTileFallStarted();
}
