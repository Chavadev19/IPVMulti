// Copyright © 2026 IpvMulti

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BalloonBurstGameMode.generated.h"

class ABalloonBurstBalloon;
class ABalloonBurstPlayerController;
class APlayerController;

/**
 * Mash-to-inflate Balloon Burst minigame. First player to burst their balloon wins.
 */
UCLASS(Blueprintable)
class IPVMULTI_API ABalloonBurstGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ABalloonBurstGameMode();

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	/** Server: called when a player's balloon reaches max pumps and bursts. */
	void NotifyBalloonBurst(APlayerController* WinnerController);

	/** Server helper used if the controller lost its balloon pointer. */
	ABalloonBurstBalloon* FindBalloonForController(APlayerController* PC) const;

protected:
	/** Warmup countdown before players can pump. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BalloonBurst|Match", meta = (ClampMin = "1"))
	int32 MatchStartDelay = 5;

	/** Pumps (button presses) required to burst a balloon. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BalloonBurst|Match", meta = (ClampMin = "1"))
	int32 PumpsToBurst = 20;

	/** Seconds to show YOU WIN / YOU LOSE before returning to the hub. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BalloonBurst|Match", meta = (ClampMin = "0.5"))
	float ReturnToHubDelay = 4.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BalloonBurst|Match")
	FString HubMapPath = TEXT("/Game/Levels/PartyGame/Lvl_MainHub");

	/** Horizontal spacing between player stations. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BalloonBurst|Arena")
	float StationSpacing = 350.0f;

	/** Balloon offset in front of the player. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BalloonBurst|Arena")
	FVector BalloonOffset = FVector(120.0f, 0.0f, 90.0f);

	UPROPERTY(EditDefaultsOnly, Category = "BalloonBurst|Arena")
	TSubclassOf<ABalloonBurstBalloon> BalloonClass;

	UPROPERTY(EditDefaultsOnly, Category = "BalloonBurst|Arena")
	bool bSpawnFloor = true;

	UPROPERTY(EditDefaultsOnly, Category = "BalloonBurst|Arena")
	FVector ArenaOrigin = FVector(0.0f, 0.0f, 100.0f);

	void TickCountdown();
	void StartBalloonBurstMatch();
	void HandleMatchEnded();
	void RefreshPlayerStations();
	void EnsureArenaFloor();
	void PlacePlayerAtStation(APlayerController* PC, int32 StationIndex);
	void SpawnBalloonForPlayer(APlayerController* PC, int32 StationIndex);
	FLinearColor GetColorForStation(int32 StationIndex) const;
	int32 CountJoinedPlayers() const;
	void SyncPlayerCount();

	UFUNCTION()
	void ReturnToHub();

	int32 CountdownRemaining = 0;
	bool bReturningToHub = false;
	bool bFloorSpawned = false;

	UPROPERTY()
	TArray<TObjectPtr<ABalloonBurstBalloon>> SpawnedBalloons;

	FTimerHandle CountdownTimerHandle;
	FTimerHandle ReturnToHubTimerHandle;
	FTimerHandle StationRefreshTimerHandle;
};
