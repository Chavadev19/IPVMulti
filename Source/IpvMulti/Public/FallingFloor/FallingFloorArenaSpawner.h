// Copyright © 2026 IpvMulti

#pragma once

#include "CoreMinimal.h"
#include "FallingFloor/FallingFloorTypes.h"
#include "GameFramework/Actor.h"
#include "FallingFloorArenaSpawner.generated.h"

class AFallingFloorTile;

/**
 * Spawns a square or hexagonal grid of FallingFloor tiles on the server.
 * Place one in Lvl_FallingFloor_Game and assign the tile class.
 */
UCLASS(Blueprintable)
class IPVMULTI_API AFallingFloorArenaSpawner : public AActor
{
	GENERATED_BODY()

public:
	AFallingFloorArenaSpawner();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FallingFloor|Arena")
	TSubclassOf<AFallingFloorTile> TileClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FallingFloor|Arena")
	EFallingFloorArenaLayout Layout = EFallingFloorArenaLayout::Hexagonal;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FallingFloor|Arena", meta = (ClampMin = "1"))
	int32 Rows = 10;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FallingFloor|Arena", meta = (ClampMin = "1"))
	int32 Columns = 10;

	/** Distance between tile centers. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FallingFloor|Arena", meta = (ClampMin = "10.0"))
	float TileSpacing = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FallingFloor|Arena")
	bool bSpawnOnBeginPlay = true;

	UFUNCTION(BlueprintCallable, Category = "FallingFloor|Arena")
	void SpawnArena();

	FVector GetTileLocation(int32 Row, int32 Column) const;
};
