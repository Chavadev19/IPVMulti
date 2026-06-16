// Copyright © 2026 IpvMulti

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RandomDoorGrid.generated.h"

class ARandomDoor;

/**
 *  Spawns a grid of doors and randomly marks 1-2 per row as openable.
 */
UCLASS()
class IPVMULTI_API ARandomDoorGrid : public AActor
{
	GENERATED_BODY()

public:
	ARandomDoorGrid();

	/** Spawns the door grid (server / standalone only). Safe to call from Blueprint. */
	UFUNCTION(BlueprintCallable, Category = "Random Door Grid")
	void GenerateDoorGrid();

protected:
	virtual void BeginPlay() override;

	/** Number of door rows (default 5). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Random Door Grid", meta = (ClampMin = "1"))
	int32 RowCount = 5;

	/** Number of doors per row (default 5). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Random Door Grid", meta = (ClampMin = "1"))
	int32 ColumnCount = 5;

	/** Minimum openable doors per row. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Random Door Grid", meta = (ClampMin = "1"))
	int32 MinOpenableDoorsPerRow = 1;

	/** Maximum openable doors per row. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Random Door Grid", meta = (ClampMin = "1"))
	int32 MaxOpenableDoorsPerRow = 2;

	/** Spacing between door centers along the row (columns). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Random Door Grid")
	float ColumnSpacing = 120.0f;

	/** Spacing between door centers along rows. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Random Door Grid")
	float RowSpacing = 220.0f;

	/** Door class to spawn. Mesh and collision are taken from this class (e.g. BP_RandomDoor). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Random Door Grid")
	TSubclassOf<ARandomDoor> DoorClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Random Door Grid")
	TArray<TObjectPtr<ARandomDoor>> SpawnedDoors;

private:
	void ClearSpawnedDoors();
	TSet<int32> PickOpenableColumnsForRow() const;
};
