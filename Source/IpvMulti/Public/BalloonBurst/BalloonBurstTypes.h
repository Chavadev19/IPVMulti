// Copyright © 2026 IpvMulti

#pragma once

#include "CoreMinimal.h"
#include "BalloonBurstTypes.generated.h"

UENUM(BlueprintType)
enum class EBalloonBurstMatchPhase : uint8
{
	WaitingToStart UMETA(DisplayName = "Waiting To Start"),
	InProgress UMETA(DisplayName = "In Progress"),
	Ended UMETA(DisplayName = "Ended")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBalloonBurstPlayersChanged, int32, PlayerCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBalloonBurstMatchPhaseChanged, EBalloonBurstMatchPhase, NewPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBalloonBurstCountdownChanged, int32, SecondsRemaining);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBalloonBurstPumpProgressChanged, int32, CurrentPumps, int32, PumpsRequired);
