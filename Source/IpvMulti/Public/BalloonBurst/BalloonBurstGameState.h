// Copyright © 2026 IpvMulti

#pragma once

#include "CoreMinimal.h"
#include "BalloonBurst/BalloonBurstTypes.h"
#include "GameFramework/GameStateBase.h"
#include "BalloonBurstGameState.generated.h"

UCLASS()
class IPVMULTI_API ABalloonBurstGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	ABalloonBurstGameState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintPure, Category = "BalloonBurst")
	int32 GetPlayerCount() const { return PlayerCount; }

	UFUNCTION(BlueprintPure, Category = "BalloonBurst")
	int32 GetCountdownSeconds() const { return CountdownSeconds; }

	UFUNCTION(BlueprintPure, Category = "BalloonBurst")
	EBalloonBurstMatchPhase GetMatchPhase() const { return MatchPhase; }

	UFUNCTION(BlueprintPure, Category = "BalloonBurst")
	bool IsMatchInProgress() const { return MatchPhase == EBalloonBurstMatchPhase::InProgress; }

	UFUNCTION(BlueprintPure, Category = "BalloonBurst")
	int32 GetPumpsRequired() const { return PumpsRequired; }

	/** Server only. */
	void SetPlayerCount(int32 NewCount);

	/** Server only. */
	void SetMatchPhase(EBalloonBurstMatchPhase NewPhase);

	/** Server only. */
	void SetCountdownSeconds(int32 NewSeconds);

	/** Server only. */
	void SetPumpsRequired(int32 NewPumpsRequired);

	UPROPERTY(BlueprintAssignable, Category = "BalloonBurst")
	FOnBalloonBurstPlayersChanged OnPlayerCountChanged;

	UPROPERTY(BlueprintAssignable, Category = "BalloonBurst")
	FOnBalloonBurstMatchPhaseChanged OnMatchPhaseChanged;

	UPROPERTY(BlueprintAssignable, Category = "BalloonBurst")
	FOnBalloonBurstCountdownChanged OnCountdownChanged;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_PlayerCount, BlueprintReadOnly, Category = "BalloonBurst")
	int32 PlayerCount = 0;

	UPROPERTY(ReplicatedUsing = OnRep_CountdownSeconds, BlueprintReadOnly, Category = "BalloonBurst")
	int32 CountdownSeconds = 0;

	UPROPERTY(ReplicatedUsing = OnRep_MatchPhase, BlueprintReadOnly, Category = "BalloonBurst")
	EBalloonBurstMatchPhase MatchPhase = EBalloonBurstMatchPhase::WaitingToStart;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "BalloonBurst")
	int32 PumpsRequired = 20;

	UFUNCTION()
	void OnRep_PlayerCount();

	UFUNCTION()
	void OnRep_MatchPhase();

	UFUNCTION()
	void OnRep_CountdownSeconds();
};
