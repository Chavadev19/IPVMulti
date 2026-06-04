// Copyright Epic Games, Inc. All Rights Reserved.

#include "IpvMultiGameMode.h"

#include "Kismet/GameplayStatics.h"

void AIpvMultiGameMode::CompleteMission(APawn* InPawn)
{
	if (InPawn == nullptr) return;
	InPawn->DisableInput(nullptr);
	UpdateViewTargetCamera(InPawn);
	OnMissionCompleted(InPawn);
}

AIpvMultiGameMode::AIpvMultiGameMode()
{
	// stub
}

void AIpvMultiGameMode::UpdateViewTargetCamera(APawn* InPawn)
{
	if (InPawn == nullptr) return;
	if (SpectatorViewClass == nullptr) return;
	
	TArray<AActor*> ReturnedActors;
	UGameplayStatics::GetAllActorsOfClass(this, SpectatorViewClass, ReturnedActors);
	if (ReturnedActors.IsEmpty()) return;
	AActor* NewViewTarget = ReturnedActors[0];
	
	APlayerController* PC = Cast<APlayerController>(InPawn->GetController());
	if (PC == nullptr) return;
	PC->SetViewTargetWithBlend(NewViewTarget, BlendTime, VTBlend_Cubic);
}
