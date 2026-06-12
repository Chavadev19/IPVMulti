// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "IpvMultiHealthWidget.generated.h"

class UProgressBar;

/**
 *  HUD health bar widget bound to CurrentHealth.
 */
UCLASS()
class UIpvMultiHealthWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	/** Updates the progress bar to reflect current health. */
	UFUNCTION(BlueprintCallable, Category = "Health")
	void UpdateHealth(float Current, float Max);

protected:

	virtual void NativeConstruct() override;

	/** Progress bar widget; optional in Blueprint child, created in C++ if missing. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> HealthProgressBar;

	void EnsureHealthProgressBar();
};
