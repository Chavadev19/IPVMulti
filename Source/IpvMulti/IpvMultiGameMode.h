// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "IpvMultiGameMode.generated.h"

/**
 *  Simple GameMode for a third person game
 */
UCLASS(abstract)
class AIpvMultiGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	void CompleteMission(APawn* InPawn);
	UFUNCTION(BlueprintImplementableEvent, Category = "Gamemode")
	void OnMissionCompleted(APawn* InPawn);
	/** Constructor */
	AIpvMultiGameMode();
	
protected:
	UPROPERTY(EditDefaultsOnly,Category="Gamemode")
	TSubclassOf<AActor> SpectatorViewClass;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Gamemode")
	float BlendTime=0.5f;
	
	void UpdateViewTargetCamera(APawn* InPawn);
};



