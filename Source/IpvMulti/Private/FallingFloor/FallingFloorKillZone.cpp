// Copyright © 2026 IpvMulti

#include "FallingFloor/FallingFloorKillZone.h"

#include "Components/BoxComponent.h"
#include "FallingFloor/FallingFloorGameMode.h"
#include "GameFramework/Pawn.h"

AFallingFloorKillZone::AFallingFloorKillZone()
{
	PrimaryActorTick.bCanEverTick = false;

	KillVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("KillVolume"));
	RootComponent = KillVolume;
	// Tall volume so fast-falling pawns cannot tunnel through between frames.
	KillVolume->SetBoxExtent(FVector(10000.0f, 10000.0f, 5000.0f));
	KillVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	KillVolume->SetCollisionObjectType(ECC_WorldDynamic);
	KillVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	KillVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	KillVolume->SetGenerateOverlapEvents(true);
	KillVolume->SetUsingAbsoluteScale(true);
	KillVolume->OnComponentBeginOverlap.AddDynamic(this, &AFallingFloorKillZone::OnOverlapBegin);
}

void AFallingFloorKillZone::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority() || OtherActor == nullptr)
	{
		return;
	}

	APawn* Pawn = Cast<APawn>(OtherActor);
	if (!Pawn)
	{
		return;
	}

	AController* Controller = Pawn->GetController();
	if (!Controller)
	{
		// Possessed pawn sometimes reports controller one frame late while falling.
		Controller = Pawn->GetOwner() ? Cast<AController>(Pawn->GetOwner()) : nullptr;
	}

	AFallingFloorGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AFallingFloorGameMode>() : nullptr;
	if (!GM || !Controller)
	{
		return;
	}

	GM->EliminatePlayer(Controller);
}
