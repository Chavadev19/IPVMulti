// Copyright © 2026 IpvMulti - Erik

#include "Erik/BalloonBurstPlayerController.h"

#include "Erik/BalloonBurstBalloon.h"
#include "Kismet/GameplayStatics.h"

ABalloonBurstPlayerController::ABalloonBurstPlayerController()
{
	bShowMouseCursor = false;
	bEnableClickEvents = false;
	bEnableMouseOverEvents = false;
}

void ABalloonBurstPlayerController::BeginPlay()
{
	Super::BeginPlay();

	ResolveTargetBalloon();

	if (FixedCameraActor)
	{
		SetViewTarget(FixedCameraActor);
	}
}

void ABalloonBurstPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (!InputComponent)
	{
		return;
	}

	InputComponent->BindKey(EKeys::SpaceBar, IE_Pressed, this, &ABalloonBurstPlayerController::OnPumpPressed);
	InputComponent->BindKey(EKeys::SpaceBar, IE_Released, this, &ABalloonBurstPlayerController::OnPumpReleased);
}

void ABalloonBurstPlayerController::ResolveTargetBalloon()
{
	if (TargetBalloon)
	{
		return;
	}

	TArray<AActor*> FoundBalloons;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABalloonBurstBalloon::StaticClass(), FoundBalloons);

	if (FoundBalloons.Num() > 0)
	{
		TargetBalloon = Cast<ABalloonBurstBalloon>(FoundBalloons[0]);
	}
}

void ABalloonBurstPlayerController::OnPumpPressed()
{
	if (!TargetBalloon)
	{
		ResolveTargetBalloon();
	}

	if (TargetBalloon)
	{
		TargetBalloon->StartInflating();
	}
}

void ABalloonBurstPlayerController::OnPumpReleased()
{
	if (TargetBalloon)
	{
		TargetBalloon->StopInflating();
	}
}
