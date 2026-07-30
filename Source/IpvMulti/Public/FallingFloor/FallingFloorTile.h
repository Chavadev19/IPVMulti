// Copyright © 2026 IpvMulti

#pragma once

#include "CoreMinimal.h"
#include "FallingFloor/FallingFloorTypes.h"
#include "GameFramework/Actor.h"
#include "FallingFloorTile.generated.h"

class UBoxComponent;
class USceneComponent;
class UStaticMeshComponent;

/**
 * Hex-A-Gone style floor tile: after a player steps on it, it warns then falls into the void.
 */
UCLASS(Blueprintable)
class IPVMULTI_API AFallingFloorTile : public AActor
{
	GENERATED_BODY()

public:
	AFallingFloorTile();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaSeconds) override;

	/** Server: if a player is already standing here when the match arms, start falling. */
	UFUNCTION(BlueprintCallable, Category = "FallingFloor|Tile")
	void CheckForStandingPlayers();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot;

	/** Query-only volume above the mesh that detects standing players. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> OverlapComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MeshComp;

	/** Delay after first step before the tile starts falling. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FallingFloor|Tile")
	float FallDelay = 0.85f;

	/** Downward speed once falling. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FallingFloor|Tile")
	float FallSpeed = 1200.0f;

	/** Destroy actor this many seconds after falling starts. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FallingFloor|Tile")
	float DestroyAfterFall = 2.5f;

	/** If true, only characters (not all pawns) trigger the tile. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FallingFloor|Tile")
	bool bCharactersOnly = true;

	UPROPERTY(ReplicatedUsing = OnRep_HasTriggered, BlueprintReadOnly, Category = "FallingFloor|Tile")
	bool bHasTriggered = false;

	UPROPERTY(ReplicatedUsing = OnRep_IsFalling, BlueprintReadOnly, Category = "FallingFloor|Tile")
	bool bIsFalling = false;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnRep_HasTriggered();

	UFUNCTION()
	void OnRep_IsFalling();

	UFUNCTION()
	void HandleMatchPhaseChanged(EFallingFloorMatchPhase NewPhase);

	bool IsValidTriggerActor(AActor* OtherActor) const;
	bool CanTriggerNow() const;
	void TriggerTile();
	void BeginFall();
	void DisableCollision();

	/** Cosmetic hook: flash / material change when stepped on. */
	UFUNCTION(BlueprintImplementableEvent, Category = "FallingFloor|Tile")
	void OnTileTriggered();

	/** Cosmetic hook when the tile starts dropping. */
	UFUNCTION(BlueprintImplementableEvent, Category = "FallingFloor|Tile")
	void OnTileFallStarted();

	FTimerHandle FallDelayTimerHandle;
	FTimerHandle OccupancyPollTimerHandle;
};
