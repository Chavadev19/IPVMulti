// Copyright © 2026 IpvMulti

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FallingFloorGameMode.generated.h"

class AFallingFloorPlayerController;

/**
 * Last-player-standing GameMode for Hex-A-Gone style Falling Floor.
 */
UCLASS(Blueprintable)
class IPVMULTI_API AFallingFloorGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AFallingFloorGameMode();

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	/** Server: eliminate a player who fell into the void. */
	UFUNCTION(BlueprintCallable, Category = "FallingFloor")
	void EliminatePlayer(AController* EliminatedController);

protected:
	/** Warmup seconds where players can move freely before tiles start falling. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FallingFloor|Match", meta = (ClampMin = "1"))
	int32 MatchStartDelay = 5;

	/** Seconds to show victory/defeat before returning to the hub. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FallingFloor|Match", meta = (ClampMin = "0.5"))
	float ReturnToHubDelay = 4.0f;

	/** Soft path to the party hub map. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FallingFloor|Match")
	FString HubMapPath = TEXT("/Game/Levels/PartyGame/Lvl_MainHub");

	/** World Z below which a pawn is considered fallen (backup if KillZone is missed). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FallingFloor|Match")
	float FallKillZ = -1500.0f;

	/** Optional spectator camera actor class used after elimination. */
	UPROPERTY(EditDefaultsOnly, Category = "FallingFloor|Match")
	TSubclassOf<AActor> SpectatorViewClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FallingFloor|Match")
	float SpectatorBlendTime = 0.75f;

	/** Destroy eliminated pawns after this delay (0 = leave them disabled in place). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FallingFloor|Match")
	float EliminatedPawnLifespan = 2.0f;

	void TickCountdown();
	void PollFallenPlayers();
	void StartFallingFloorMatch();
	void CheckWinCondition();
	void HandleMatchEnded();

	UFUNCTION()
	void ReturnToHub();

	void SwitchToSpectatorView(APlayerController* PC);
	void RespawnAtPlayerStart(AController* Controller);
	int32 CountActivePlayers() const;

	int32 CountdownRemaining = 0;
	bool bReturningToHub = false;

	FTimerHandle CountdownTimerHandle;
	FTimerHandle ReturnToHubTimerHandle;
	FTimerHandle FallPollTimerHandle;
};
