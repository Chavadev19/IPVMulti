// Copyright © 2026 IpvMulti

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "IpvMultiGameStateBase.generated.h"

/**
 * 
 */
UCLASS()
class IPVMULTI_API AIpvMultiGameStateBase : public AGameStateBase
{
	GENERATED_BODY()
	
	
	public:
	UFUNCTION(NetMulticast, Reliable)
	void MulticastOnMissionCompleted(APawn* InPawn, bool bIsMissionSucceed);
};
