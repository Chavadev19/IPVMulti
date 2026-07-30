// Copyright © 2026 IpvMulti

#include "FallingFloor/FallingFloorPlayerState.h"
#include "Net/UnrealNetwork.h"

AFallingFloorPlayerState::AFallingFloorPlayerState()
{
	bReplicates = true;
}

void AFallingFloorPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFallingFloorPlayerState, bIsEliminated);
}

void AFallingFloorPlayerState::SetEliminated(bool bNewEliminated)
{
	if (!HasAuthority())
	{
		return;
	}

	bIsEliminated = bNewEliminated;
	OnRep_IsEliminated();
}

void AFallingFloorPlayerState::OnRep_IsEliminated()
{
}
