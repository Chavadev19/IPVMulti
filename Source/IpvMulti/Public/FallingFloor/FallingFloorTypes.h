// Copyright © 2026 IpvMulti

#pragma once

#include "CoreMinimal.h"
#include "FallingFloorTypes.generated.h"

UENUM(BlueprintType)
enum class EFallingFloorMatchPhase : uint8
{
	WaitingToStart UMETA(DisplayName = "Waiting To Start"),
	InProgress UMETA(DisplayName = "In Progress"),
	Ended UMETA(DisplayName = "Ended")
};

UENUM(BlueprintType)
enum class EFallingFloorArenaLayout : uint8
{
	Square UMETA(DisplayName = "Square Grid"),
	Hexagonal UMETA(DisplayName = "Hexagonal Grid")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFallingFloorPlayersRemainingChanged, int32, PlayersRemaining);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFallingFloorMatchPhaseChanged, EFallingFloorMatchPhase, NewPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFallingFloorCountdownChanged, int32, SecondsRemaining);
