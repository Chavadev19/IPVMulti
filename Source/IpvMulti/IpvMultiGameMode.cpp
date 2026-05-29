// Copyright Epic Games, Inc. All Rights Reserved.

#include "IpvMultiGameMode.h"

void AIpvMultiGameMode::CompleteMission(APawn* InPawn)
{
	if (InPawn == nullptr) return;
	InPawn->DisableInput(nullptr);
	OnMissionCompleted(InPawn);
}

AIpvMultiGameMode::AIpvMultiGameMode()
{
	// stub
}
