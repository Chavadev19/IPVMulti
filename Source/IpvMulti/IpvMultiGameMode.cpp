// Copyright Epic Games, Inc. All Rights Reserved.

#include "IpvMultiGameMode.h"

#include "IpvMultiPlayerController.h"
#include "Game/IpvMultiGameStateBase.h"
#include "Kismet/GameplayStatics.h"

void AIpvMultiGameMode::CompleteMission(APawn* InPawn, bool bIsMissionSucced)
{
	if (InPawn == nullptr) return;
	//InPawn->DisableInput(nullptr);
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (PC)
		{
			AIpvMultiPlayerController* IpvMultiPC = Cast<AIpvMultiPlayerController>(PC);
			if (IpvMultiPC)
			{
				APawn* MyPawn = IpvMultiPC->GetPawn();
				IpvMultiPC->OnMissionCompleted(MyPawn, bIsMissionSucced); 
				UpdateViewTargetCamera(MyPawn);
			}
		}
	}
	
	AIpvMultiGameStateBase* GS = GetGameState<AIpvMultiGameStateBase>();
	if (GS)
	{
		GS->MulticastOnMissionCompleted(InPawn, bIsMissionSucced);
	}
	OnMissionCompleted(InPawn, bIsMissionSucced);
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
