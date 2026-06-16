// Copyright © 2026 IpvMulti

#include "Actors/RandomDoor.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"

ARandomDoor::ARandomDoor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(false);

	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
	RootComponent = DoorMesh;
	DoorMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));

	TouchTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("TouchTrigger"));
	TouchTrigger->SetupAttachment(DoorMesh);
	TouchTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TouchTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
	TouchTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void ARandomDoor::BeginPlay()
{
	Super::BeginPlay();

	UpdateTouchTriggerFromMesh();
	TouchTrigger->OnComponentBeginOverlap.AddDynamic(this, &ARandomDoor::OnTouchTriggerOverlap);

	if (bIsOpened)
	{
		ApplyOpenedVisuals();
	}
}

void ARandomDoor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARandomDoor, RowIndex);
	DOREPLIFETIME(ARandomDoor, ColumnIndex);
	DOREPLIFETIME(ARandomDoor, bIsOpenable);
	DOREPLIFETIME(ARandomDoor, bIsOpened);
}

void ARandomDoor::InitializeDoor(int32 InRowIndex, int32 InColumnIndex, bool bInOpenable)
{
	RowIndex = InRowIndex;
	ColumnIndex = InColumnIndex;
	bIsOpenable = bInOpenable;
	bIsOpened = false;
}

void ARandomDoor::UpdateTouchTriggerFromMesh()
{
	if (DoorMesh == nullptr || TouchTrigger == nullptr)
	{
		return;
	}

	const UStaticMesh* StaticMesh = DoorMesh->GetStaticMesh();
	if (StaticMesh == nullptr)
	{
		return;
	}

	const FBox LocalBox = StaticMesh->GetBoundingBox();
	const FVector Scale = DoorMesh->GetRelativeScale3D();
	const FVector Origin = LocalBox.GetCenter() * Scale;
	const FVector BoxExtent = LocalBox.GetExtent() * Scale;

	const float TouchPadding = 8.0f;
	TouchTrigger->SetRelativeLocation(Origin);
	TouchTrigger->SetBoxExtent(BoxExtent + FVector(TouchPadding));
}

void ARandomDoor::OnTouchTriggerOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!bIsOpenable || bIsOpened)
	{
		return;
	}

	const ACharacter* Character = Cast<ACharacter>(OtherActor);
	if (Character == nullptr || !Character->IsPlayerControlled())
	{
		return;
	}

	if (HasAuthority())
	{
		OpenDoor();
	}
}

void ARandomDoor::OpenDoor()
{
	if (bIsOpened)
	{
		return;
	}

	bIsOpened = true;
	ApplyOpenedVisuals();
	MulticastOpenDoor();
}

void ARandomDoor::ApplyOpenedVisuals()
{
	SetActorEnableCollision(false);
	SetActorHiddenInGame(true);

	if (DoorMesh)
	{
		DoorMesh->SetVisibility(false, true);
		DoorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (TouchTrigger)
	{
		TouchTrigger->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void ARandomDoor::OnRep_IsOpened()
{
	if (bIsOpened)
	{
		ApplyOpenedVisuals();
	}
}

void ARandomDoor::MulticastOpenDoor_Implementation()
{
	ApplyOpenedVisuals();
}
