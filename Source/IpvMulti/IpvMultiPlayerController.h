// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "IpvMultiPlayerController.generated.h"

class UInputMappingContext;
class UUserWidget;
class UIpvMultiHealthWidget;

/**
 *  Basic PlayerController class for a third person game
 *  Manages input mappings
 */
UCLASS(abstract)
class AIpvMultiPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	AIpvMultiPlayerController();

protected:

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category ="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;

	/** Mobile controls widget to spawn */
	UPROPERTY(EditAnywhere, Category="Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	/** Pointer to the mobile controls widget */
	UPROPERTY()
	TObjectPtr<UUserWidget> MobileControlsWidget;

	/** Health HUD widget class */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UIpvMultiHealthWidget> HealthWidgetClass;

	/** Active health HUD widget for the local player */
	UPROPERTY()
	TObjectPtr<UIpvMultiHealthWidget> HealthWidget;

	/** If true, the player will use UMG touch controls even if not playing on mobile platforms */
	UPROPERTY(EditAnywhere, Config, Category = "Input|Touch Controls")
	bool bForceTouchControls = false;

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	virtual void OnPossess(APawn* InPawn) override;

	/** Input mapping context setup */
	virtual void SetupInputComponent() override;

	/** Returns true if the player should use UMG touch controls */
	bool ShouldUseTouchControls() const;

	void EnsureHealthWidget();

public:

	/** Updates the local health HUD display. */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateHealthDisplay(float Current, float Max);
};
