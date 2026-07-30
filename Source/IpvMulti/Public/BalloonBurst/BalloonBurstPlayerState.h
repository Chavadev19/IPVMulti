// Copyright © 2026 IpvMulti

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "BalloonBurstPlayerState.generated.h"

UCLASS()
class IPVMULTI_API ABalloonBurstPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	ABalloonBurstPlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintPure, Category = "BalloonBurst")
	int32 GetPumpCount() const { return PumpCount; }

	UFUNCTION(BlueprintPure, Category = "BalloonBurst")
	bool HasBurstBalloon() const { return bHasBurstBalloon; }

	UFUNCTION(BlueprintPure, Category = "BalloonBurst")
	bool DidWin() const { return bDidWin; }

	UFUNCTION(BlueprintPure, Category = "BalloonBurst")
	int32 GetStationIndex() const { return StationIndex; }

	/** Server only. */
	void SetPumpCount(int32 NewCount);

	/** Server only. */
	void SetHasBurstBalloon(bool bBurst);

	/** Server only. */
	void SetDidWin(bool bWon);

	/** Server only. */
	void SetStationIndex(int32 NewIndex);

protected:
	UPROPERTY(ReplicatedUsing = OnRep_PumpCount, BlueprintReadOnly, Category = "BalloonBurst")
	int32 PumpCount = 0;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "BalloonBurst")
	bool bHasBurstBalloon = false;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "BalloonBurst")
	bool bDidWin = false;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "BalloonBurst")
	int32 StationIndex = INDEX_NONE;

	UFUNCTION()
	void OnRep_PumpCount();
};
