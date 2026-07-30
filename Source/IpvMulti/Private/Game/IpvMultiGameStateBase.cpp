// Copyright © 2026 IpvMulti


#include "Game/IpvMultiGameStateBase.h"

void AIpvMultiGameStateBase::MulticastOnMissionCompleted_Implementation(APawn* InPawn, bool bIsMissionSucceed)
{
	if (APlayerController* PC = Cast<APlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		if (APawn* MyPawn = PC->GetPawn())
		{
			MyPawn->DisableInput(nullptr);
		}
	}
}
