// Copyright © 2026 IpvMulti

#pragma once

#include "CoreMinimal.h"
#include "FallingFloor/FallingFloorTypes.h"
#include "GameFramework/PlayerController.h"
#include "FallingFloorPlayerController.generated.h"

class UFallingFloorUI;
class UInputMappingContext;

/**
 * PlayerController for Falling Floor: owns the minimal HUD and win/lose banners.
 */
UCLASS(Blueprintable)
class IPVMULTI_API AFallingFloorPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AFallingFloorPlayerController();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;

	UFUNCTION(Client, Reliable)
	void ClientMatchStarted(int32 PlayersAlive);

	UFUNCTION(Client, Reliable)
	void ClientUpdatePlayersRemaining(int32 PlayersAlive);

	UFUNCTION(Client, Reliable)
	void ClientUpdateCountdown(int32 SecondsRemaining);

	UFUNCTION(Client, Reliable)
	void ClientNotifyMatchPhase(EFallingFloorMatchPhase NewPhase);

	UFUNCTION(Client, Reliable)
	void ClientShowVictory();

	UFUNCTION(Client, Reliable)
	void ClientShowDefeat();

protected:
	UPROPERTY(EditAnywhere, Category = "Input|Input Mappings")
	TArray<TObjectPtr<UInputMappingContext>> DefaultMappingContexts;

	UPROPERTY(EditAnywhere, Category = "Input|Input Mappings")
	TArray<TObjectPtr<UInputMappingContext>> MobileExcludedMappingContexts;

	/** Falling Floor HUD widget class (create WBP child of FallingFloorUI in editor). */
	UPROPERTY(EditDefaultsOnly, Category = "FallingFloor|UI")
	TSubclassOf<UFallingFloorUI> FallingFloorUIClass;

	UPROPERTY(Transient)
	TObjectPtr<UFallingFloorUI> FallingFloorUIInstance;

	void EnsureDefaultMappingContexts();
	void ApplyGameplayInputMode();
	void CreateHUD();
	void BindGameStateEvents();
	void StripLegacyPlayerHUD();

	UFUNCTION()
	void HandlePlayersRemainingChanged(int32 PlayersRemaining);

	UFUNCTION()
	void HandleMatchPhaseChanged(EFallingFloorMatchPhase NewPhase);

	UFUNCTION()
	void HandleCountdownChanged(int32 SecondsRemaining);

	bool bResultScreenShown = false;
	bool bMappingContextsAdded = false;
	int32 LegacyHudStripCount = 0;

	FTimerHandle LegacyHudStripTimerHandle;
};
