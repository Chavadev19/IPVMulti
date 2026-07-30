// Copyright © 2026 IpvMulti

#include "BalloonBurst/BalloonBurstGameMode.h"

#include "BalloonBurst/BalloonBurstBalloon.h"
#include "BalloonBurst/BalloonBurstGameState.h"
#include "BalloonBurst/BalloonBurstPlayerController.h"
#include "BalloonBurst/BalloonBurstPlayerState.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerStart.h"
#include "IpvMulti.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

ABalloonBurstGameMode::ABalloonBurstGameMode()
{
	GameStateClass = ABalloonBurstGameState::StaticClass();
	PlayerStateClass = ABalloonBurstPlayerState::StaticClass();
	PlayerControllerClass = ABalloonBurstPlayerController::StaticClass();
	BalloonClass = ABalloonBurstBalloon::StaticClass();
	bUseSeamlessTravel = false;
}

void ABalloonBurstGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	if (UClass* PawnClass = LoadClass<APawn>(
		nullptr,
		TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter.BP_ThirdPersonCharacter_C")))
	{
		DefaultPawnClass = PawnClass;
	}
}

void ABalloonBurstGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
	{
		return;
	}

	ABalloonBurstGameState* GS = GetGameState<ABalloonBurstGameState>();
	if (!GS)
	{
		return;
	}

	bReturningToHub = false;
	GS->SetPumpsRequired(PumpsToBurst);
	GS->SetMatchPhase(EBalloonBurstMatchPhase::WaitingToStart);
	CountdownRemaining = FMath::Max(1, MatchStartDelay);
	GS->SetCountdownSeconds(CountdownRemaining);

	bBurstWindowOpen = false;
	bResultsFinalized = false;
	PendingBurstControllers.Reset();

	EnsureArenaFloor();
	RefreshPlayerStations();
	SyncPlayerCount();

	GetWorldTimerManager().SetTimer(
		CountdownTimerHandle,
		this,
		&ABalloonBurstGameMode::TickCountdown,
		1.0f,
		true);

	// Late-joining PIE clients may possess a frame or two after BeginPlay.
	GetWorldTimerManager().SetTimer(
		StationRefreshTimerHandle,
		this,
		&ABalloonBurstGameMode::RefreshPlayerStations,
		0.5f,
		true);
}

void ABalloonBurstGameMode::TickCountdown()
{
	if (!HasAuthority())
	{
		return;
	}

	ABalloonBurstGameState* GS = GetGameState<ABalloonBurstGameState>();
	if (!GS || GS->GetMatchPhase() != EBalloonBurstMatchPhase::WaitingToStart)
	{
		GetWorldTimerManager().ClearTimer(CountdownTimerHandle);
		return;
	}

	CountdownRemaining = FMath::Max(0, CountdownRemaining - 1);
	GS->SetCountdownSeconds(CountdownRemaining);

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (ABalloonBurstPlayerController* PC = Cast<ABalloonBurstPlayerController>(It->Get()))
		{
			PC->ClientUpdateCountdown(CountdownRemaining);
		}
	}

	if (CountdownRemaining <= 0)
	{
		GetWorldTimerManager().ClearTimer(CountdownTimerHandle);
		StartBalloonBurstMatch();
	}
}

void ABalloonBurstGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	ABalloonBurstGameState* GS = GetGameState<ABalloonBurstGameState>();
	if (!GS || !HasAuthority())
	{
		return;
	}

	if (GS->GetMatchPhase() == EBalloonBurstMatchPhase::Ended || bReturningToHub)
	{
		return;
	}

	RefreshPlayerStations();
	SyncPlayerCount();

	if (ABalloonBurstPlayerController* PC = Cast<ABalloonBurstPlayerController>(NewPlayer))
	{
		PC->ClientUpdatePlayerCount(GS->GetPlayerCount());
		PC->ClientNotifyMatchPhase(GS->GetMatchPhase());
		PC->ClientUpdateCountdown(GS->GetCountdownSeconds());
		PC->ClientUpdatePumpProgress(0, GS->GetPumpsRequired());

		if (GS->IsMatchInProgress())
		{
			PC->ClientMatchStarted(GS->GetPlayerCount());
		}
	}
}

void ABalloonBurstGameMode::Logout(AController* Exiting)
{
	if (HasAuthority() && Exiting)
	{
		if (ABalloonBurstPlayerController* PC = Cast<ABalloonBurstPlayerController>(Exiting))
		{
			if (ABalloonBurstBalloon* Balloon = PC->GetOwnedBalloon())
			{
				SpawnedBalloons.Remove(Balloon);
				Balloon->Destroy();
				PC->SetOwnedBalloon(nullptr);
			}
		}

		SyncPlayerCount();
	}

	Super::Logout(Exiting);
}

void ABalloonBurstGameMode::StartBalloonBurstMatch()
{
	ABalloonBurstGameState* GS = GetGameState<ABalloonBurstGameState>();
	if (!GS || !HasAuthority())
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(StationRefreshTimerHandle);

	GS->SetCountdownSeconds(0);
	RefreshPlayerStations();
	SyncPlayerCount();

	const int32 Players = GS->GetPlayerCount();
	GS->SetMatchPhase(EBalloonBurstMatchPhase::InProgress);

	UE_LOG(LogIpvMulti, Log, TEXT("BalloonBurst match started with %d players (pumps to burst=%d)"),
		Players, PumpsToBurst);

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (ABalloonBurstPlayerController* PC = Cast<ABalloonBurstPlayerController>(It->Get()))
		{
			PC->ClientMatchStarted(Players);
			PC->ClientUpdatePumpProgress(0, PumpsToBurst);
		}
	}
}

void ABalloonBurstGameMode::NotifyBalloonBurst(APlayerController* BurstController)
{
	if (!HasAuthority() || !BurstController || bReturningToHub || bResultsFinalized)
	{
		return;
	}

	ABalloonBurstGameState* GS = GetGameState<ABalloonBurstGameState>();
	if (!GS || !GS->IsMatchInProgress())
	{
		return;
	}

	// Already counted this player for the current burst window.
	if (PendingBurstControllers.Contains(BurstController))
	{
		return;
	}

	PendingBurstControllers.Add(BurstController);

	UE_LOG(LogIpvMulti, Warning, TEXT("BalloonBurst burst candidate: %s (pending=%d)"),
		BurstController->PlayerState ? *BurstController->PlayerState->GetPlayerName() : TEXT("?"),
		PendingBurstControllers.Num());

	// Keep accepting near-simultaneous burst RPCs for a short grace period so host RTT
	// doesn't always win when players effectively tied.
	if (!bBurstWindowOpen)
	{
		bBurstWindowOpen = true;
		GetWorldTimerManager().ClearTimer(TieResolveTimerHandle);
		GetWorldTimerManager().SetTimer(
			TieResolveTimerHandle,
			this,
			&ABalloonBurstGameMode::ResolveBurstWindow,
			FMath::Max(0.05f, TieGraceSeconds),
			false);
	}
}

void ABalloonBurstGameMode::ResolveBurstWindow()
{
	if (!HasAuthority() || bReturningToHub || bResultsFinalized)
	{
		return;
	}

	TArray<APlayerController*> BurstControllers;
	BurstControllers.Reserve(PendingBurstControllers.Num());
	for (APlayerController* PC : PendingBurstControllers)
	{
		if (IsValid(PC))
		{
			BurstControllers.Add(PC);
		}
	}

	PendingBurstControllers.Reset();
	bBurstWindowOpen = false;
	GetWorldTimerManager().ClearTimer(TieResolveTimerHandle);

	if (BurstControllers.Num() == 0)
	{
		return;
	}

	FinalizeMatchResults(BurstControllers);
}

void ABalloonBurstGameMode::FinalizeMatchResults(const TArray<APlayerController*>& BurstControllers)
{
	ABalloonBurstGameState* GS = GetGameState<ABalloonBurstGameState>();
	if (!GS || !HasAuthority() || bReturningToHub || bResultsFinalized)
	{
		return;
	}

	if (!GS->IsMatchInProgress())
	{
		return;
	}

	// Seal immediately so a late burst RPC cannot reopen another window mid-finalize.
	bResultsFinalized = true;

	const bool bIsDraw = BurstControllers.Num() >= 2;

	if (bIsDraw)
	{
		UE_LOG(LogIpvMulti, Warning, TEXT("BalloonBurst DRAW between %d players"), BurstControllers.Num());
	}
	else
	{
		const APlayerState* WinnerPS = BurstControllers[0] ? BurstControllers[0]->PlayerState : nullptr;
		UE_LOG(LogIpvMulti, Warning, TEXT("BalloonBurst winner: %s"),
			WinnerPS ? *WinnerPS->GetPlayerName() : TEXT("?"));
	}

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ABalloonBurstPlayerController* PC = Cast<ABalloonBurstPlayerController>(It->Get());
		if (!PC)
		{
			continue;
		}

		const bool bDidBurst = BurstControllers.Contains(PC);
		if (ABalloonBurstPlayerState* PS = PC->GetPlayerState<ABalloonBurstPlayerState>())
		{
			PS->SetDidWin(bDidBurst && !bIsDraw);
			if (bDidBurst)
			{
				PS->SetHasBurstBalloon(true);
			}
		}

		if (bIsDraw && bDidBurst)
		{
			PC->ClientShowDraw();
		}
		else if (!bIsDraw && bDidBurst)
		{
			PC->ClientShowVictory();
		}
		else
		{
			PC->ClientShowDefeat();
		}

		if (APawn* Pawn = PC->GetPawn())
		{
			Pawn->DisableInput(PC);
		}
	}

	HandleMatchEnded();
}

void ABalloonBurstGameMode::HandleMatchEnded()
{
	ABalloonBurstGameState* GS = GetGameState<ABalloonBurstGameState>();
	if (!GS || !HasAuthority() || bReturningToHub)
	{
		return;
	}

	bReturningToHub = true;
	bBurstWindowOpen = false;
	PendingBurstControllers.Reset();
	GS->SetMatchPhase(EBalloonBurstMatchPhase::Ended);
	GetWorldTimerManager().ClearTimer(CountdownTimerHandle);
	GetWorldTimerManager().ClearTimer(StationRefreshTimerHandle);
	GetWorldTimerManager().ClearTimer(TieResolveTimerHandle);

	UE_LOG(LogIpvMulti, Warning, TEXT("BalloonBurst match ended. Returning to hub in %.1fs"), ReturnToHubDelay);

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (ABalloonBurstPlayerController* PC = Cast<ABalloonBurstPlayerController>(It->Get()))
		{
			PC->ClientNotifyMatchPhase(EBalloonBurstMatchPhase::Ended);
		}
	}

	GetWorldTimerManager().ClearTimer(ReturnToHubTimerHandle);
	GetWorldTimerManager().SetTimer(
		ReturnToHubTimerHandle,
		this,
		&ABalloonBurstGameMode::ReturnToHub,
		ReturnToHubDelay,
		false);
}

void ABalloonBurstGameMode::ReturnToHub()
{
	UWorld* World = GetWorld();
	if (!HasAuthority() || !World)
	{
		return;
	}

	FString TravelURL = HubMapPath;
	TravelURL.TrimStartAndEndInline();
	if (TravelURL.IsEmpty())
	{
		TravelURL = TEXT("/Game/Levels/PartyGame/Lvl_MainHub");
	}

	if (!TravelURL.Contains(TEXT("listen"), ESearchCase::IgnoreCase))
	{
		TravelURL += TravelURL.Contains(TEXT("?")) ? TEXT("&listen") : TEXT("?listen");
	}

	UE_LOG(LogIpvMulti, Warning, TEXT("BalloonBurst ServerTravel -> %s"), *TravelURL);
	World->ServerTravel(TravelURL, false);
}

void ABalloonBurstGameMode::RefreshPlayerStations()
{
	if (!HasAuthority() || bReturningToHub)
	{
		return;
	}

	ABalloonBurstGameState* GS = GetGameState<ABalloonBurstGameState>();
	if (!GS || GS->GetMatchPhase() == EBalloonBurstMatchPhase::Ended)
	{
		return;
	}

	EnsureArenaFloor();

	int32 StationIndex = 0;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC || !PC->GetPawn())
		{
			continue;
		}

		ABalloonBurstPlayerController* BBPC = Cast<ABalloonBurstPlayerController>(PC);
		ABalloonBurstPlayerState* PS = PC->GetPlayerState<ABalloonBurstPlayerState>();
		const bool bNeedsSetup = !PS || PS->GetStationIndex() == INDEX_NONE || !BBPC || BBPC->GetOwnedBalloon() == nullptr;

		if (bNeedsSetup)
		{
			PlacePlayerAtStation(PC, StationIndex);
			SpawnBalloonForPlayer(PC, StationIndex);
			if (PS)
			{
				PS->SetStationIndex(StationIndex);
			}
		}

		++StationIndex;
	}

	SyncPlayerCount();
}

void ABalloonBurstGameMode::PlacePlayerAtStation(APlayerController* PC, int32 StationIndex)
{
	if (!PC)
	{
		return;
	}

	APawn* Pawn = PC->GetPawn();
	if (!Pawn)
	{
		return;
	}

	const float CenterOffset = (CountJoinedPlayers() - 1) * 0.5f * StationSpacing;
	const FVector StationLocation = ArenaOrigin + FVector(0.0f, StationIndex * StationSpacing - CenterOffset, 0.0f);
	const FRotator StationRotation = FRotator(0.0f, 0.0f, 0.0f);

	Pawn->SetActorLocationAndRotation(StationLocation, StationRotation, false, nullptr, ETeleportType::TeleportPhysics);
	PC->SetControlRotation(StationRotation);

	if (ACharacter* StationCharacter = Cast<ACharacter>(Pawn))
	{
		StationCharacter->JumpMaxCount = 0;
		if (UCharacterMovementComponent* Move = StationCharacter->GetCharacterMovement())
		{
			Move->StopMovementImmediately();
			Move->DisableMovement();
		}
	}
}

void ABalloonBurstGameMode::SpawnBalloonForPlayer(APlayerController* PC, int32 StationIndex)
{
	if (!PC || !GetWorld())
	{
		return;
	}

	ABalloonBurstPlayerController* BBPC = Cast<ABalloonBurstPlayerController>(PC);
	if (BBPC && BBPC->GetOwnedBalloon())
	{
		return;
	}

	TSubclassOf<ABalloonBurstBalloon> ClassToSpawn = BalloonClass;
	if (!ClassToSpawn)
	{
		ClassToSpawn = ABalloonBurstBalloon::StaticClass();
	}

	APawn* Pawn = PC->GetPawn();
	const FVector BaseLocation = Pawn
		? Pawn->GetActorLocation()
		: ArenaOrigin + FVector(0.0f, StationIndex * StationSpacing, 0.0f);
	const FVector BalloonLocation = BaseLocation + BalloonOffset;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Params.Owner = PC;

	ABalloonBurstBalloon* Balloon = GetWorld()->SpawnActor<ABalloonBurstBalloon>(
		ClassToSpawn,
		BalloonLocation,
		FRotator::ZeroRotator,
		Params);

	if (!Balloon)
	{
		UE_LOG(LogIpvMulti, Error, TEXT("BalloonBurst failed to spawn balloon for station %d"), StationIndex);
		return;
	}

	ABalloonBurstPlayerState* PS = PC->GetPlayerState<ABalloonBurstPlayerState>();
	Balloon->InitializeBalloon(PS, StationIndex, PumpsToBurst, GetColorForStation(StationIndex));
	SpawnedBalloons.Add(Balloon);

	if (BBPC)
	{
		BBPC->SetOwnedBalloon(Balloon);
	}
}

ABalloonBurstBalloon* ABalloonBurstGameMode::FindBalloonForController(APlayerController* PC) const
{
	if (!PC)
	{
		return nullptr;
	}

	const ABalloonBurstPlayerState* PS = PC->GetPlayerState<ABalloonBurstPlayerState>();
	for (ABalloonBurstBalloon* Balloon : SpawnedBalloons)
	{
		if (Balloon && Balloon->GetOwnerPlayerState() == PS)
		{
			return Balloon;
		}
	}

	return nullptr;
}

FLinearColor ABalloonBurstGameMode::GetColorForStation(int32 StationIndex) const
{
	static const FLinearColor Colors[] = {
		FLinearColor(0.95f, 0.2f, 0.25f),
		FLinearColor(0.2f, 0.45f, 0.95f),
		FLinearColor(0.2f, 0.85f, 0.35f),
		FLinearColor(0.95f, 0.85f, 0.15f),
		FLinearColor(0.85f, 0.35f, 0.9f),
		FLinearColor(0.95f, 0.55f, 0.15f)
	};

	const int32 NumColors = UE_ARRAY_COUNT(Colors);
	return Colors[FMath::Abs(StationIndex) % NumColors];
}

void ABalloonBurstGameMode::EnsureArenaFloor()
{
	if (!HasAuthority() || !bSpawnFloor || bFloorSpawned || !GetWorld())
	{
		return;
	}

	UStaticMesh* PlaneMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (!PlaneMesh)
	{
		return;
	}

	// Deferred spawn so mesh/scale are set before the actor is networked to clients.
	// AStaticMeshActor defaults to bReplicates=false, so without this only the host sees the floor.
	const FTransform FloorTransform(FRotator::ZeroRotator, ArenaOrigin - FVector(0.0f, 0.0f, 100.0f));
	AStaticMeshActor* Floor = GetWorld()->SpawnActorDeferred<AStaticMeshActor>(
		AStaticMeshActor::StaticClass(),
		FloorTransform,
		nullptr,
		nullptr,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	if (!Floor)
	{
		return;
	}

	Floor->SetReplicates(true);
	Floor->bAlwaysRelevant = true;
	Floor->SetReplicateMovement(false);

	if (UStaticMeshComponent* Mesh = Floor->GetStaticMeshComponent())
	{
		Mesh->SetIsReplicated(true);
		Mesh->SetMobility(EComponentMobility::Movable);
		Mesh->SetStaticMesh(PlaneMesh);
		Mesh->SetRelativeScale3D(FVector(20.0f, 20.0f, 1.0f));
		Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Mesh->SetCollisionProfileName(TEXT("BlockAll"));
	}

	Floor->FinishSpawning(FloorTransform);
	bFloorSpawned = true;
}

int32 ABalloonBurstGameMode::CountJoinedPlayers() const
{
	int32 Count = 0;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (PC && PC->GetPawn())
		{
			++Count;
		}
	}
	return FMath::Max(1, Count);
}

void ABalloonBurstGameMode::SyncPlayerCount()
{
	ABalloonBurstGameState* GS = GetGameState<ABalloonBurstGameState>();
	if (!GS || !HasAuthority())
	{
		return;
	}

	int32 Count = 0;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (It->Get())
		{
			++Count;
		}
	}

	GS->SetPlayerCount(Count);

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (ABalloonBurstPlayerController* PC = Cast<ABalloonBurstPlayerController>(It->Get()))
		{
			PC->ClientUpdatePlayerCount(Count);
		}
	}
}
