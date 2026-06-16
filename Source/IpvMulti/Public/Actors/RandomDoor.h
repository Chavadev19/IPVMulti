// Copyright © 2026 IpvMulti

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RandomDoor.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

/**
 *  Single door tile in the random door grid. Only openable doors disappear when touched.
 *  Visual mesh and blocking collision come from the Door Class (e.g. BP_RandomDoor).
 */
UCLASS()
class IPVMULTI_API ARandomDoor : public AActor
{
	GENERATED_BODY()

public:
	ARandomDoor();

	UFUNCTION(BlueprintPure, Category = "Random Door")
	int32 GetRowIndex() const { return RowIndex; }

	UFUNCTION(BlueprintPure, Category = "Random Door")
	int32 GetColumnIndex() const { return ColumnIndex; }

	UFUNCTION(BlueprintPure, Category = "Random Door")
	bool IsOpenable() const { return bIsOpenable; }

	UFUNCTION(BlueprintPure, Category = "Random Door")
	bool IsOpened() const { return bIsOpened; }

	void InitializeDoor(int32 InRowIndex, int32 InColumnIndex, bool bInOpenable);

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> DoorMesh;

	/** Overlap-only volume sized from DoorMesh bounds at runtime. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> TouchTrigger;

	UPROPERTY(Replicated)
	int32 RowIndex = 0;

	UPROPERTY(Replicated)
	int32 ColumnIndex = 0;

	UPROPERTY(Replicated)
	bool bIsOpenable = false;

	UPROPERTY(ReplicatedUsing = OnRep_IsOpened)
	bool bIsOpened = false;

	UFUNCTION()
	void OnRep_IsOpened();

	UFUNCTION()
	void OnTouchTriggerOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	void OpenDoor();
	void ApplyOpenedVisuals();
	void UpdateTouchTriggerFromMesh();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastOpenDoor();
};
