// Copyright © 2026 IpvMulti

#include "FallingFloor/FallingFloorGameState.h"
#include "Net/UnrealNetwork.h"

AFallingFloorGameState::AFallingFloorGameState()
{
	bReplicates = true;
}

void AFallingFloorGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFallingFloorGameState, PlayersRemaining);
	DOREPLIFETIME(AFallingFloorGameState, InitialPlayerCount);
	DOREPLIFETIME(AFallingFloorGameState, CountdownSeconds);
	DOREPLIFETIME(AFallingFloorGameState, MatchPhase);
}

void AFallingFloorGameState::SetPlayersRemaining(int32 NewCount)
{
	if (!HasAuthority())
	{
		return;
	}

	PlayersRemaining = FMath::Max(0, NewCount);
	OnRep_PlayersRemaining();
}

void AFallingFloorGameState::SetInitialPlayerCount(int32 NewCount)
{
	if (!HasAuthority())
	{
		return;
	}

	InitialPlayerCount = FMath::Max(0, NewCount);
}

void AFallingFloorGameState::SetMatchPhase(EFallingFloorMatchPhase NewPhase)
{
	if (!HasAuthority())
	{
		return;
	}

	MatchPhase = NewPhase;
	OnRep_MatchPhase();
}

void AFallingFloorGameState::SetCountdownSeconds(int32 NewSeconds)
{
	if (!HasAuthority())
	{
		return;
	}

	CountdownSeconds = FMath::Max(0, NewSeconds);
	OnRep_CountdownSeconds();
}

void AFallingFloorGameState::OnRep_PlayersRemaining()
{
	OnPlayersRemainingChanged.Broadcast(PlayersRemaining);
}

void AFallingFloorGameState::OnRep_MatchPhase()
{
	OnMatchPhaseChanged.Broadcast(MatchPhase);
}

void AFallingFloorGameState::OnRep_CountdownSeconds()
{
	OnCountdownChanged.Broadcast(CountdownSeconds);
}
