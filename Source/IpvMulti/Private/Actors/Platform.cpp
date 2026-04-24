// Copyright © 2026 IpvMulti


#include "Public/Actors/Platform.h"

#include "Components/BoxComponent.h"


// Sets default values
APlatform::APlatform()
{
	OverlapComp = CreateDefaultSubobject<UBoxComponent>("BoxComp");
	RootComponent = OverlapComp;
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>("MeshComp");
	MeshComp->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void APlatform::BeginPlay()
{
	Super::BeginPlay();
	
}


