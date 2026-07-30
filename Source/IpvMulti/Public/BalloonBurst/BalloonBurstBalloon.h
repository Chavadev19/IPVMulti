// Copyright © 2026 IpvMulti

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BalloonBurstBalloon.generated.h"

class ABalloonBurstPlayerState;
class UStaticMeshComponent;
class UMaterialInstanceDynamic;

/**
 * Visual balloon that inflates with each pump and bursts when full.
 */
UCLASS(Blueprintable)
class IPVMULTI_API ABalloonBurstBalloon : public AActor
{
	GENERATED_BODY()

public:
	ABalloonBurstBalloon();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;

	void InitializeBalloon(ABalloonBurstPlayerState* InOwnerState, int32 InStationIndex, int32 InPumpsRequired, FLinearColor InColor);

	/** Server: one button press = one pump of air. Returns true if this pump burst the balloon. */
	bool ServerAddPump();

	UFUNCTION(BlueprintPure, Category = "BalloonBurst")
	int32 GetCurrentPumps() const { return CurrentPumps; }

	UFUNCTION(BlueprintPure, Category = "BalloonBurst")
	int32 GetPumpsRequired() const { return PumpsRequired; }

	UFUNCTION(BlueprintPure, Category = "BalloonBurst")
	bool HasBurst() const { return bHasBurst; }

	ABalloonBurstPlayerState* GetOwnerPlayerState() const { return OwnerPlayerState; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> BalloonMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> StringMesh;

	UPROPERTY(EditDefaultsOnly, Category = "BalloonBurst")
	float MinScale = 0.35f;

	UPROPERTY(EditDefaultsOnly, Category = "BalloonBurst")
	float MaxScale = 1.75f;

	UPROPERTY(EditDefaultsOnly, Category = "BalloonBurst")
	float BurstScale = 2.4f;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentPumps)
	int32 CurrentPumps = 0;

	UPROPERTY(Replicated)
	int32 PumpsRequired = 20;

	UPROPERTY(ReplicatedUsing = OnRep_HasBurst)
	bool bHasBurst = false;

	UPROPERTY(ReplicatedUsing = OnRep_BalloonColor)
	FLinearColor BalloonColor = FLinearColor::Red;

	UPROPERTY(Replicated)
	TObjectPtr<ABalloonBurstPlayerState> OwnerPlayerState;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> BalloonMID;

	UFUNCTION()
	void OnRep_CurrentPumps();

	UFUNCTION()
	void OnRep_HasBurst();

	UFUNCTION()
	void OnRep_BalloonColor();

	void ApplyVisualScale() const;
	void ApplyBalloonColor();
	void PlayBurstVisuals();
};
