// Copyright © 2026 IpvMulti


#include "Actors/ObjectiveZone.h"

#include "IpvMultiCharacter.h"
#include "IpvMultiGameMode.h"
#include "Components/BoxComponent.h"
#include "Components/DecalComponent.h"


// Sets default values
AObjectiveZone::AObjectiveZone()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	OverlapComp = CreateDefaultSubobject<UBoxComponent>(TEXT("OverlapComp"));
	OverlapComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	OverlapComp->SetCollisionResponseToChannels(ECR_Ignore);
	OverlapComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	OverlapComp->SetBoxExtent(FVector(200.0f));
	RootComponent = OverlapComp;
	OverlapComp->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::HandleOverlap);
	DecalComp = CreateDefaultSubobject<UDecalComponent>(TEXT("DecalComp"));
	DecalComp->DecalSize = FVector(200.0f);
	DecalComp->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AObjectiveZone::BeginPlay()
{
	Super::BeginPlay();
	
}

void AObjectiveZone::HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AIpvMultiCharacter* MyPawn = Cast<AIpvMultiCharacter>(OtherActor);
	if (MyPawn == nullptr) return;
	if (MyPawn->bIsCarryingObjective)
	{
		AIpvMultiGameMode* GM = Cast<AIpvMultiGameMode>(GetWorld()->GetAuthGameMode());
		if (GM == nullptr) return;
		GM->CompleteMission(MyPawn, true);
		
	}
}


