// Copyright © 2026 IpvMulti

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FallingFloor/FallingFloorTypes.h"
#include "FallingFloorUI.generated.h"

class UTextBlock;
class UBorder;

/**
 * Minimal Falling Floor HUD with a built-in remaining-players counter and win/lose banners.
 * Create WBP_FallingFloorHUD parented to this class and (optionally) restyle via BindWidget overrides:
 *   PlayersRemainingText, VictoryBanner, DefeatBanner, StatusText
 */
UCLASS(Blueprintable)
class IPVMULTI_API UFallingFloorUI : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category = "FallingFloor|UI")
	void UpdatePlayersRemaining(int32 PlayersRemaining);

	UFUNCTION(BlueprintCallable, Category = "FallingFloor|UI")
	void UpdateCountdown(int32 SecondsRemaining);

	UFUNCTION(BlueprintCallable, Category = "FallingFloor|UI")
	void NotifyMatchPhase(EFallingFloorMatchPhase NewPhase);

	UFUNCTION(BlueprintCallable, Category = "FallingFloor|UI")
	void OnMatchStarted(int32 PlayersAlive);

	UFUNCTION(BlueprintCallable, Category = "FallingFloor|UI")
	void ShowVictory();

	UFUNCTION(BlueprintCallable, Category = "FallingFloor|UI")
	void ShowDefeat();

	/** Optional Blueprint cosmetics after the C++ defaults run. */
	UFUNCTION(BlueprintImplementableEvent, Category = "FallingFloor|UI")
	void BP_UpdatePlayersRemaining(int32 PlayersRemaining);

	UFUNCTION(BlueprintImplementableEvent, Category = "FallingFloor|UI")
	void BP_UpdateCountdown(int32 SecondsRemaining);

	UFUNCTION(BlueprintImplementableEvent, Category = "FallingFloor|UI")
	void BP_NotifyMatchPhase(EFallingFloorMatchPhase NewPhase);

	UFUNCTION(BlueprintImplementableEvent, Category = "FallingFloor|UI")
	void BP_OnMatchStarted(int32 PlayersAlive);

	UFUNCTION(BlueprintImplementableEvent, Category = "FallingFloor|UI")
	void BP_ShowVictory();

	UFUNCTION(BlueprintImplementableEvent, Category = "FallingFloor|UI")
	void BP_ShowDefeat();

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "FallingFloor|UI")
	TObjectPtr<UTextBlock> PlayersRemainingText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "FallingFloor|UI")
	TObjectPtr<UTextBlock> StatusText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "FallingFloor|UI")
	TObjectPtr<UTextBlock> CountdownText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "FallingFloor|UI")
	TObjectPtr<UBorder> VictoryBanner;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "FallingFloor|UI")
	TObjectPtr<UBorder> DefeatBanner;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> RuntimePlayersText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> RuntimeStatusText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> RuntimeCountdownText;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> RuntimeVictoryBanner;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> RuntimeDefeatBanner;

	UTextBlock* GetPlayersText() const;
	UTextBlock* GetStatus() const;
	UTextBlock* GetCountdown() const;
	UBorder* GetVictory() const;
	UBorder* GetDefeat() const;

	bool bBuiltRuntimeLayout = false;
};
