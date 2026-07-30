// Copyright © 2026 IpvMulti

#pragma once

#include "CoreMinimal.h"
#include "BalloonBurst/BalloonBurstTypes.h"
#include "GameFramework/PlayerController.h"
#include "BalloonBurstPlayerController.generated.h"

class ABalloonBurstBalloon;
class UBalloonBurstUI;
class UInputAction;
class UInputMappingContext;

/**
 * Owns HUD, pump input (Space) and win/lose client RPCs.
 */
UCLASS(Blueprintable)
class IPVMULTI_API ABalloonBurstPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ABalloonBurstPlayerController();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;

	void SetOwnedBalloon(ABalloonBurstBalloon* InBalloon);
	ABalloonBurstBalloon* GetOwnedBalloon() const { return OwnedBalloon.Get(); }

	UFUNCTION(Client, Reliable)
	void ClientMatchStarted(int32 PlayerCount);

	UFUNCTION(Client, Reliable)
	void ClientUpdatePlayerCount(int32 PlayerCount);

	UFUNCTION(Client, Reliable)
	void ClientUpdateCountdown(int32 SecondsRemaining);

	UFUNCTION(Client, Reliable)
	void ClientNotifyMatchPhase(EBalloonBurstMatchPhase NewPhase);

	UFUNCTION(Client, Reliable)
	void ClientUpdatePumpProgress(int32 CurrentPumps, int32 PumpsRequired);

	UFUNCTION(Client, Reliable)
	void ClientShowVictory();

	UFUNCTION(Client, Reliable)
	void ClientShowDefeat();

protected:
	UPROPERTY(EditAnywhere, Category = "Input|Input Mappings")
	TArray<TObjectPtr<UInputMappingContext>> DefaultMappingContexts;

	UPROPERTY(EditAnywhere, Category = "Input|Input Mappings")
	TArray<TObjectPtr<UInputMappingContext>> MobileExcludedMappingContexts;

	UPROPERTY(EditDefaultsOnly, Category = "BalloonBurst|Input")
	TObjectPtr<UInputAction> PumpAction;

	UPROPERTY(EditDefaultsOnly, Category = "BalloonBurst|UI")
	TSubclassOf<UBalloonBurstUI> BalloonBurstUIClass;

	UPROPERTY(Transient)
	TObjectPtr<UBalloonBurstUI> BalloonBurstUIInstance;

	UPROPERTY(Transient)
	TWeakObjectPtr<ABalloonBurstBalloon> OwnedBalloon;

	void EnsureDefaultMappingContexts();
	void EnsurePumpAction();
	void ApplyGameplayInputMode();
	void CreateHUD();
	void BindGameStateEvents();
	void StripLegacyPlayerHUD();
	void ConfigurePawnForMinigame(APawn* InPawn);
	void HandlePumpPressed();

	UFUNCTION(Server, Reliable)
	void ServerRequestPump();

	UFUNCTION()
	void HandlePlayerCountChanged(int32 PlayerCount);

	UFUNCTION()
	void HandleMatchPhaseChanged(EBalloonBurstMatchPhase NewPhase);

	UFUNCTION()
	void HandleCountdownChanged(int32 SecondsRemaining);

	bool bResultScreenShown = false;
	bool bMappingContextsAdded = false;
	bool bPumpBound = false;
	int32 LegacyHudStripCount = 0;

	FTimerHandle LegacyHudStripTimerHandle;
};
