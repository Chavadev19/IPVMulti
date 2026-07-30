// Copyright © 2026 IpvMulti

#include "BalloonBurst/BalloonBurstPlayerState.h"
#include "Net/UnrealNetwork.h"

ABalloonBurstPlayerState::ABalloonBurstPlayerState()
{
	bReplicates = true;
}

void ABalloonBurstPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABalloonBurstPlayerState, PumpCount);
	DOREPLIFETIME(ABalloonBurstPlayerState, bHasBurstBalloon);
	DOREPLIFETIME(ABalloonBurstPlayerState, bDidWin);
	DOREPLIFETIME(ABalloonBurstPlayerState, StationIndex);
}

void ABalloonBurstPlayerState::SetPumpCount(int32 NewCount)
{
	if (!HasAuthority())
	{
		return;
	}

	PumpCount = FMath::Max(0, NewCount);
	OnRep_PumpCount();
}

void ABalloonBurstPlayerState::SetHasBurstBalloon(bool bBurst)
{
	if (!HasAuthority())
	{
		return;
	}

	bHasBurstBalloon = bBurst;
}

void ABalloonBurstPlayerState::SetDidWin(bool bWon)
{
	if (!HasAuthority())
	{
		return;
	}

	bDidWin = bWon;
}

void ABalloonBurstPlayerState::SetStationIndex(int32 NewIndex)
{
	if (!HasAuthority())
	{
		return;
	}

	StationIndex = NewIndex;
}

void ABalloonBurstPlayerState::OnRep_PumpCount()
{
}
