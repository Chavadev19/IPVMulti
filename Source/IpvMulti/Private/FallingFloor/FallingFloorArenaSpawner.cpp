// Copyright © 2026 IpvMulti

#include "FallingFloor/FallingFloorArenaSpawner.h"

#include "FallingFloor/FallingFloorTile.h"
#include "IpvMulti.h"

AFallingFloorArenaSpawner::AFallingFloorArenaSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
	TileClass = AFallingFloorTile::StaticClass();
}

void AFallingFloorArenaSpawner::BeginPlay()
{
	Super::BeginPlay();

	if (bSpawnOnBeginPlay && HasAuthority())
	{
		SpawnArena();
	}
}

void AFallingFloorArenaSpawner::SpawnArena()
{
	if (!HasAuthority() || !GetWorld())
	{
		return;
	}

	if (!TileClass || TileClass == AFallingFloorTile::StaticClass())
	{
		if (UClass* BPTile = LoadClass<AFallingFloorTile>(
			nullptr,
			TEXT("/Game/Blueprints/FallingFloor/BP_FallingFloorTile.BP_FallingFloorTile_C")))
		{
			TileClass = BPTile;
		}
		else if (!TileClass)
		{
			TileClass = AFallingFloorTile::StaticClass();
		}
	}

	if (!TileClass)
	{
		UE_LOG(LogIpvMulti, Error, TEXT("FallingFloorArenaSpawner: TileClass is null, cannot spawn arena."));
		return;
	}

	const FVector Origin = GetActorLocation();
	const FRotator Rotation = GetActorRotation();
	int32 SpawnedCount = 0;

	for (int32 Row = 0; Row < Rows; ++Row)
	{
		for (int32 Column = 0; Column < Columns; ++Column)
		{
			const FVector LocalOffset = GetTileLocation(Row, Column);
			const FVector WorldLocation = Origin + Rotation.RotateVector(LocalOffset);

			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			Params.Owner = this;

			if (GetWorld()->SpawnActor<AFallingFloorTile>(TileClass, WorldLocation, Rotation, Params))
			{
				++SpawnedCount;
			}
		}
	}

	UE_LOG(LogIpvMulti, Log, TEXT("FallingFloorArenaSpawner: spawned %d tiles at %s"),
		SpawnedCount, *Origin.ToCompactString());
}

FVector AFallingFloorArenaSpawner::GetTileLocation(int32 Row, int32 Column) const
{
	const float CenterRow = (Rows - 1) * 0.5f;
	const float CenterCol = (Columns - 1) * 0.5f;

	if (Layout == EFallingFloorArenaLayout::Hexagonal)
	{
		const float X = (Column - CenterCol) * TileSpacing + ((Row & 1) ? TileSpacing * 0.5f : 0.0f);
		const float Y = (Row - CenterRow) * TileSpacing * 0.8660254f;
		return FVector(X, Y, 0.0f);
	}

	const float X = (Column - CenterCol) * TileSpacing;
	const float Y = (Row - CenterRow) * TileSpacing;
	return FVector(X, Y, 0.0f);
}
