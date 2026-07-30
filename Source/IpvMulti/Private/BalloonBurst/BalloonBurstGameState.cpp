// Copyright © 2026 IpvMulti

#include "BalloonBurst/BalloonBurstGameState.h"
#include "Net/UnrealNetwork.h"

ABalloonBurstGameState::ABalloonBurstGameState()
{
	bReplicates = true;
}

void ABalloonBurstGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABalloonBurstGameState, PlayerCount);
	DOREPLIFETIME(ABalloonBurstGameState, CountdownSeconds);
	DOREPLIFETIME(ABalloonBurstGameState, MatchPhase);
	DOREPLIFETIME(ABalloonBurstGameState, PumpsRequired);
}

void ABalloonBurstGameState::SetPlayerCount(int32 NewCount)
{
	if (!HasAuthority())
	{
		return;
	}

	PlayerCount = FMath::Max(0, NewCount);
	OnRep_PlayerCount();
}

void ABalloonBurstGameState::SetMatchPhase(EBalloonBurstMatchPhase NewPhase)
{
	if (!HasAuthority())
	{
		return;
	}

	MatchPhase = NewPhase;
	OnRep_MatchPhase();
}

void ABalloonBurstGameState::SetCountdownSeconds(int32 NewSeconds)
{
	if (!HasAuthority())
	{
		return;
	}

	CountdownSeconds = FMath::Max(0, NewSeconds);
	OnRep_CountdownSeconds();
}

void ABalloonBurstGameState::SetPumpsRequired(int32 NewPumpsRequired)
{
	if (!HasAuthority())
	{
		return;
	}

	PumpsRequired = FMath::Max(1, NewPumpsRequired);
}

void ABalloonBurstGameState::OnRep_PlayerCount()
{
	OnPlayerCountChanged.Broadcast(PlayerCount);
}

void ABalloonBurstGameState::OnRep_MatchPhase()
{
	OnMatchPhaseChanged.Broadcast(MatchPhase);
}

void ABalloonBurstGameState::OnRep_CountdownSeconds()
{
	OnCountdownChanged.Broadcast(CountdownSeconds);
}
