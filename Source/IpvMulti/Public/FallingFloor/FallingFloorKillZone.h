// Copyright © 2026 IpvMulti

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FallingFloorKillZone.generated.h"

class UBoxComponent;

/**
 * Volume under the arena. Overlapping players are eliminated by the FallingFloor GameMode.
 */
UCLASS(Blueprintable)
class IPVMULTI_API AFallingFloorKillZone : public AActor
{
	GENERATED_BODY()

public:
	AFallingFloorKillZone();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> KillVolume;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
