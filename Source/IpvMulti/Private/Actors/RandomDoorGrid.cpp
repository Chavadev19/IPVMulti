// Copyright © 2026 IpvMulti

#include "Actors/RandomDoorGrid.h"

#include "Actors/RandomDoor.h"
#include "Engine/World.h"

ARandomDoorGrid::ARandomDoorGrid()
{
	PrimaryActorTick.bCanEverTick = false;
	DoorClass = ARandomDoor::StaticClass();
}

void ARandomDoorGrid::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		GenerateDoorGrid();
	}
}

void ARandomDoorGrid::GenerateDoorGrid()
{
	if (!HasAuthority() || DoorClass == nullptr)
	{
		return;
	}

	ClearSpawnedDoors();

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	const FVector Origin = GetActorLocation();
	const FRotator Facing = GetActorRotation();

	for (int32 Row = 0; Row < RowCount; ++Row)
	{
		const TSet<int32> OpenableColumns = PickOpenableColumnsForRow();

		for (int32 Column = 0; Column < ColumnCount; ++Column)
		{
			const FVector LocalOffset(Column * ColumnSpacing, Row * RowSpacing, 0.0f);
			const FVector SpawnLocation = Origin + Facing.RotateVector(LocalOffset);
			const FTransform SpawnTransform(Facing, SpawnLocation);

			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			ARandomDoor* Door = World->SpawnActor<ARandomDoor>(DoorClass, SpawnTransform, SpawnParams);
			if (Door == nullptr)
			{
				continue;
			}

			const bool bOpenable = OpenableColumns.Contains(Column);
			Door->InitializeDoor(Row, Column, bOpenable);
			SpawnedDoors.Add(Door);
		}
	}
}

void ARandomDoorGrid::ClearSpawnedDoors()
{
	for (ARandomDoor* Door : SpawnedDoors)
	{
		if (IsValid(Door))
		{
			Door->Destroy();
		}
	}

	SpawnedDoors.Reset();
}

TSet<int32> ARandomDoorGrid::PickOpenableColumnsForRow() const
{
	TArray<int32> Columns;
	Columns.Reserve(ColumnCount);

	for (int32 Column = 0; Column < ColumnCount; ++Column)
	{
		Columns.Add(Column);
	}

	const int32 SafeMinOpenable = FMath::Clamp(MinOpenableDoorsPerRow, 1, ColumnCount);
	const int32 SafeMaxOpenable = FMath::Clamp(MaxOpenableDoorsPerRow, SafeMinOpenable, ColumnCount);
	const int32 OpenableCount = FMath::RandRange(SafeMinOpenable, SafeMaxOpenable);

	for (int32 Index = 0; Index < OpenableCount; ++Index)
	{
		const int32 SwapIndex = FMath::RandRange(Index, ColumnCount - 1);
		if (SwapIndex != Index)
		{
			Columns.Swap(Index, SwapIndex);
		}
	}

	TSet<int32> OpenableColumns;
	for (int32 Index = 0; Index < OpenableCount; ++Index)
	{
		OpenableColumns.Add(Columns[Index]);
	}

	return OpenableColumns;
}
