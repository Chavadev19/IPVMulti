// Copyright © 2026 IpvMulti

#pragma once

#include "CoreMinimal.h"
#include "FallingFloor/FallingFloorTypes.h"
#include "GameFramework/GameStateBase.h"
#include "FallingFloorGameState.generated.h"

UCLASS()
class IPVMULTI_API AFallingFloorGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	AFallingFloorGameState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintPure, Category = "FallingFloor")
	int32 GetPlayersRemaining() const { return PlayersRemaining; }

	UFUNCTION(BlueprintPure, Category = "FallingFloor")
	int32 GetInitialPlayerCount() const { return InitialPlayerCount; }

	UFUNCTION(BlueprintPure, Category = "FallingFloor")
	int32 GetCountdownSeconds() const { return CountdownSeconds; }

	UFUNCTION(BlueprintPure, Category = "FallingFloor")
	EFallingFloorMatchPhase GetMatchPhase() const { return MatchPhase; }

	UFUNCTION(BlueprintPure, Category = "FallingFloor")
	bool IsMatchInProgress() const { return MatchPhase == EFallingFloorMatchPhase::InProgress; }

	/** Server only. */
	void SetPlayersRemaining(int32 NewCount);

	/** Server only. */
	void SetInitialPlayerCount(int32 NewCount);

	/** Server only. */
	void SetMatchPhase(EFallingFloorMatchPhase NewPhase);

	/** Server only. */
	void SetCountdownSeconds(int32 NewSeconds);

	UPROPERTY(BlueprintAssignable, Category = "FallingFloor")
	FOnFallingFloorPlayersRemainingChanged OnPlayersRemainingChanged;

	UPROPERTY(BlueprintAssignable, Category = "FallingFloor")
	FOnFallingFloorMatchPhaseChanged OnMatchPhaseChanged;

	UPROPERTY(BlueprintAssignable, Category = "FallingFloor")
	FOnFallingFloorCountdownChanged OnCountdownChanged;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_PlayersRemaining, BlueprintReadOnly, Category = "FallingFloor")
	int32 PlayersRemaining = 0;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "FallingFloor")
	int32 InitialPlayerCount = 0;

	UPROPERTY(ReplicatedUsing = OnRep_CountdownSeconds, BlueprintReadOnly, Category = "FallingFloor")
	int32 CountdownSeconds = 0;

	UPROPERTY(ReplicatedUsing = OnRep_MatchPhase, BlueprintReadOnly, Category = "FallingFloor")
	EFallingFloorMatchPhase MatchPhase = EFallingFloorMatchPhase::WaitingToStart;

	UFUNCTION()
	void OnRep_PlayersRemaining();

	UFUNCTION()
	void OnRep_MatchPhase();

	UFUNCTION()
	void OnRep_CountdownSeconds();
};
