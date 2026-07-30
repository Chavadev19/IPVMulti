// Copyright © 2026 IpvMulti

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "FallingFloorPlayerState.generated.h"

UCLASS()
class IPVMULTI_API AFallingFloorPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AFallingFloorPlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintPure, Category = "FallingFloor")
	bool IsEliminated() const { return bIsEliminated; }

	/** Server only. */
	void SetEliminated(bool bNewEliminated);

protected:
	UPROPERTY(ReplicatedUsing = OnRep_IsEliminated, BlueprintReadOnly, Category = "FallingFloor")
	bool bIsEliminated = false;

	UFUNCTION()
	void OnRep_IsEliminated();
};
