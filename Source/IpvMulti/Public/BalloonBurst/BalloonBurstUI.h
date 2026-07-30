// Copyright © 2026 IpvMulti

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BalloonBurst/BalloonBurstTypes.h"
#include "BalloonBurstUI.generated.h"

class UTextBlock;
class UBorder;

/**
 * Balloon Burst HUD with instructions, countdown, START, progress and YOU WIN / YOU LOSE.
 * Works fully from C++ via RebuildWidget when no WBP is assigned.
 */
UCLASS(Blueprintable)
class IPVMULTI_API UBalloonBurstUI : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category = "BalloonBurst|UI")
	void UpdatePlayerCount(int32 PlayerCount);

	UFUNCTION(BlueprintCallable, Category = "BalloonBurst|UI")
	void UpdateCountdown(int32 SecondsRemaining);

	UFUNCTION(BlueprintCallable, Category = "BalloonBurst|UI")
	void NotifyMatchPhase(EBalloonBurstMatchPhase NewPhase);

	UFUNCTION(BlueprintCallable, Category = "BalloonBurst|UI")
	void OnMatchStarted(int32 PlayerCount);

	UFUNCTION(BlueprintCallable, Category = "BalloonBurst|UI")
	void ShowStartBanner();

	UFUNCTION(BlueprintCallable, Category = "BalloonBurst|UI")
	void UpdatePumpProgress(int32 CurrentPumps, int32 PumpsRequired);

	UFUNCTION(BlueprintCallable, Category = "BalloonBurst|UI")
	void ShowVictory();

	UFUNCTION(BlueprintCallable, Category = "BalloonBurst|UI")
	void ShowDefeat();

	UFUNCTION(BlueprintImplementableEvent, Category = "BalloonBurst|UI")
	void BP_UpdatePlayerCount(int32 PlayerCount);

	UFUNCTION(BlueprintImplementableEvent, Category = "BalloonBurst|UI")
	void BP_UpdateCountdown(int32 SecondsRemaining);

	UFUNCTION(BlueprintImplementableEvent, Category = "BalloonBurst|UI")
	void BP_NotifyMatchPhase(EBalloonBurstMatchPhase NewPhase);

	UFUNCTION(BlueprintImplementableEvent, Category = "BalloonBurst|UI")
	void BP_OnMatchStarted(int32 PlayerCount);

	UFUNCTION(BlueprintImplementableEvent, Category = "BalloonBurst|UI")
	void BP_ShowStartBanner();

	UFUNCTION(BlueprintImplementableEvent, Category = "BalloonBurst|UI")
	void BP_UpdatePumpProgress(int32 CurrentPumps, int32 PumpsRequired);

	UFUNCTION(BlueprintImplementableEvent, Category = "BalloonBurst|UI")
	void BP_ShowVictory();

	UFUNCTION(BlueprintImplementableEvent, Category = "BalloonBurst|UI")
	void BP_ShowDefeat();

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "BalloonBurst|UI")
	TObjectPtr<UTextBlock> PlayersText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "BalloonBurst|UI")
	TObjectPtr<UTextBlock> InstructionsText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "BalloonBurst|UI")
	TObjectPtr<UTextBlock> StatusText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "BalloonBurst|UI")
	TObjectPtr<UTextBlock> CountdownText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "BalloonBurst|UI")
	TObjectPtr<UTextBlock> ProgressText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "BalloonBurst|UI")
	TObjectPtr<UBorder> VictoryBanner;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "BalloonBurst|UI")
	TObjectPtr<UBorder> DefeatBanner;

	UPROPERTY(Transient) TObjectPtr<UTextBlock> RuntimePlayersText;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> RuntimeInstructionsText;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> RuntimeStatusText;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> RuntimeCountdownText;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> RuntimeProgressText;
	UPROPERTY(Transient) TObjectPtr<UBorder> RuntimeVictoryBanner;
	UPROPERTY(Transient) TObjectPtr<UBorder> RuntimeDefeatBanner;

	UTextBlock* GetPlayers() const;
	UTextBlock* GetInstructions() const;
	UTextBlock* GetStatus() const;
	UTextBlock* GetCountdown() const;
	UTextBlock* GetProgress() const;
	UBorder* GetVictory() const;
	UBorder* GetDefeat() const;

	void HideStartBanner();

	bool bBuiltRuntimeLayout = false;
	bool bShowingStart = false;
	FTimerHandle StartBannerTimerHandle;
};
