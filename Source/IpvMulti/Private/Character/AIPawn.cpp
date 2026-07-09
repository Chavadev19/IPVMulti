// Copyright © 2026 IpvMulti


#include "Character/AIPawn.h"

#include "Perception/PawnSensingComponent.h"


// Sets default values
AAIPawn::AAIPawn()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	PawnSensingComp = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensingComp"));
}

// Called when the game starts or when spawned
void AAIPawn::BeginPlay()
{
	Super::BeginPlay();
	
	OriginalRotation = GetActorRotation();
	
}

void AAIPawn::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	PawnSensingComp->OnSeePawn.AddDynamic(this, &AAIPawn::HandleOnSeenPawn);
	PawnSensingComp->OnHearNoise.AddDynamic(this, &AAIPawn::HandleOnHearNoise);
}

void AAIPawn::HandleOnSeenPawn(APawn* InPawn)
{
	if (InPawn == nullptr) return;
	DrawDebugSphere(GetWorld(), InPawn->GetActorLocation(), 35.0f, 12, FColor::Blue, false, 5.0f);
}

void AAIPawn::HandleOnHearNoise(APawn* InInstigator, const FVector& Location, float Volume)
{
	DrawDebugSphere(GetWorld(), Location, 35.0f, 12, FColor::Green, false, 5.0f);
	FVector Direction = Location - GetActorLocation();
	Direction.Normalize();
	
	FRotator NewLookAt = FRotationMatrix::MakeFromX(Direction).Rotator();
	NewLookAt.Pitch = 0.0f;
	NewLookAt.Roll = 0.0f;
	
	SetActorRotation(NewLookAt);
	
	GetWorldTimerManager().ClearTimer(TimerHandle_ResetOrientation);
	
	GetWorldTimerManager().SetTimer(TimerHandle_ResetOrientation, this, &ThisClass::ResetOrientation, 3.0f);
}

void AAIPawn::ResetOrientation()
{
	SetActorRotation(OriginalRotation);
}

// Called every frame
void AAIPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AAIPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

