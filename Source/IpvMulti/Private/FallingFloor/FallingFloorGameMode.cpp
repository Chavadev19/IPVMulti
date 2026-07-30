// Copyright © 2026 IpvMulti

#include "FallingFloor/FallingFloorGameMode.h"

#include "FallingFloor/FallingFloorGameState.h"
#include "FallingFloor/FallingFloorPlayerController.h"
#include "FallingFloor/FallingFloorPlayerState.h"
#include "FallingFloor/FallingFloorTile.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerState.h"
#include "IpvMulti.h"
#include "Kismet/GameplayStatics.h"

AFallingFloorGameMode::AFallingFloorGameMode()
{
	GameStateClass = AFallingFloorGameState::StaticClass();
	PlayerStateClass = AFallingFloorPlayerState::StaticClass();
	PlayerControllerClass = AFallingFloorPlayerController::StaticClass();
	bUseSeamlessTravel = false;
}

void AFallingFloorGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	if (UClass* PCClass = LoadClass<APlayerController>(
		nullptr,
		TEXT("/Game/Blueprints/FallingFloor/BP_FallingFloorPlayerController.BP_FallingFloorPlayerController_C")))
	{
		PlayerControllerClass = PCClass;
	}

	if (UClass* PawnClass = LoadClass<APawn>(
		nullptr,
		TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter.BP_ThirdPersonCharacter_C")))
	{
		DefaultPawnClass = PawnClass;
	}
}

void AFallingFloorGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
	{
		return;
	}

	AFallingFloorGameState* GS = GetGameState<AFallingFloorGameState>();
	if (!GS)
	{
		return;
	}

	bReturningToHub = false;
	GS->SetMatchPhase(EFallingFloorMatchPhase::WaitingToStart);
	CountdownRemaining = FMath::Max(1, MatchStartDelay);
	GS->SetCountdownSeconds(CountdownRemaining);

	GetWorldTimerManager().SetTimer(
		CountdownTimerHandle,
		this,
		&AFallingFloorGameMode::TickCountdown,
		1.0f,
		true);

	// Backup for thin KillZones: continuously catch anyone below FallKillZ.
	GetWorldTimerManager().SetTimer(
		FallPollTimerHandle,
		this,
		&AFallingFloorGameMode::PollFallenPlayers,
		0.25f,
		true);
}

void AFallingFloorGameMode::TickCountdown()
{
	if (!HasAuthority())
	{
		return;
	}

	AFallingFloorGameState* GS = GetGameState<AFallingFloorGameState>();
	if (!GS || GS->GetMatchPhase() != EFallingFloorMatchPhase::WaitingToStart)
	{
		GetWorldTimerManager().ClearTimer(CountdownTimerHandle);
		return;
	}

	CountdownRemaining = FMath::Max(0, CountdownRemaining - 1);
	GS->SetCountdownSeconds(CountdownRemaining);

	if (CountdownRemaining <= 0)
	{
		GetWorldTimerManager().ClearTimer(CountdownTimerHandle);
		StartFallingFloorMatch();
	}
}

void AFallingFloorGameMode::PollFallenPlayers()
{
	if (!HasAuthority() || bReturningToHub)
	{
		return;
	}

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC)
		{
			continue;
		}

		APawn* Pawn = PC->GetPawn();
		if (!Pawn)
		{
			continue;
		}

		if (Pawn->GetActorLocation().Z < FallKillZ)
		{
			EliminatePlayer(PC);
		}
	}
}

void AFallingFloorGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	AFallingFloorGameState* GS = GetGameState<AFallingFloorGameState>();
	if (!GS || !HasAuthority())
	{
		return;
	}

	if (GS->IsMatchInProgress() || GS->GetMatchPhase() == EFallingFloorMatchPhase::Ended)
	{
		EliminatePlayer(NewPlayer);
		return;
	}

	if (AFallingFloorPlayerController* PC = Cast<AFallingFloorPlayerController>(NewPlayer))
	{
		PC->ClientUpdatePlayersRemaining(GS->GetPlayersRemaining());
		PC->ClientNotifyMatchPhase(GS->GetMatchPhase());
		PC->ClientUpdateCountdown(GS->GetCountdownSeconds());
	}
}

void AFallingFloorGameMode::Logout(AController* Exiting)
{
	AFallingFloorGameState* GS = GetGameState<AFallingFloorGameState>();
	if (HasAuthority() && GS && GS->IsMatchInProgress())
	{
		if (AFallingFloorPlayerState* PS = Exiting ? Exiting->GetPlayerState<AFallingFloorPlayerState>() : nullptr)
		{
			if (!PS->IsEliminated())
			{
				PS->SetEliminated(true);
				GS->SetPlayersRemaining(GS->GetPlayersRemaining() - 1);
				CheckWinCondition();
			}
		}
	}

	Super::Logout(Exiting);
}

void AFallingFloorGameMode::StartFallingFloorMatch()
{
	AFallingFloorGameState* GS = GetGameState<AFallingFloorGameState>();
	if (!GS || !HasAuthority())
	{
		return;
	}

	GS->SetCountdownSeconds(0);

	const int32 AliveCount = CountActivePlayers();
	GS->SetInitialPlayerCount(AliveCount);
	GS->SetPlayersRemaining(AliveCount);
	GS->SetMatchPhase(EFallingFloorMatchPhase::InProgress);

	UE_LOG(LogIpvMulti, Log, TEXT("FallingFloor match started with %d players"), AliveCount);

	TArray<AActor*> Tiles;
	UGameplayStatics::GetAllActorsOfClass(this, AFallingFloorTile::StaticClass(), Tiles);
	for (AActor* TileActor : Tiles)
	{
		if (AFallingFloorTile* Tile = Cast<AFallingFloorTile>(TileActor))
		{
			Tile->CheckForStandingPlayers();
		}
	}

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (AFallingFloorPlayerController* PC = Cast<AFallingFloorPlayerController>(It->Get()))
		{
			PC->ClientMatchStarted(AliveCount);
		}
	}
}

void AFallingFloorGameMode::EliminatePlayer(AController* EliminatedController)
{
	if (!HasAuthority() || EliminatedController == nullptr || bReturningToHub)
	{
		return;
	}

	AFallingFloorGameState* GS = GetGameState<AFallingFloorGameState>();
	if (!GS)
	{
		return;
	}

	// During warmup, just put them back on the arena so they can keep moving.
	if (GS->GetMatchPhase() == EFallingFloorMatchPhase::WaitingToStart)
	{
		RespawnAtPlayerStart(EliminatedController);
		return;
	}

	if (GS->GetMatchPhase() == EFallingFloorMatchPhase::Ended)
	{
		return;
	}

	AFallingFloorPlayerState* PS = EliminatedController->GetPlayerState<AFallingFloorPlayerState>();
	if (!PS || PS->IsEliminated())
	{
		return;
	}

	PS->SetEliminated(true);

	if (GS->IsMatchInProgress())
	{
		GS->SetPlayersRemaining(GS->GetPlayersRemaining() - 1);
	}

	UE_LOG(LogIpvMulti, Log, TEXT("FallingFloor player eliminated. Remaining=%d"), GS->GetPlayersRemaining());

	if (AFallingFloorPlayerController* PC = Cast<AFallingFloorPlayerController>(EliminatedController))
	{
		PC->ClientShowDefeat();
		PC->ClientUpdatePlayersRemaining(GS->GetPlayersRemaining());
		SwitchToSpectatorView(PC);
	}

	if (APawn* Pawn = EliminatedController->GetPawn())
	{
		if (APlayerController* PC = Cast<APlayerController>(EliminatedController))
		{
			Pawn->DisableInput(PC);
		}

		// Stop endless falling immediately.
		Pawn->SetActorHiddenInGame(true);
		Pawn->SetActorEnableCollision(false);
		if (USceneComponent* Root = Pawn->GetRootComponent())
		{
			Root->SetVisibility(false, true);
		}

		if (EliminatedPawnLifespan > 0.0f)
		{
			Pawn->SetLifeSpan(EliminatedPawnLifespan);
		}
	}

	CheckWinCondition();
}

void AFallingFloorGameMode::RespawnAtPlayerStart(AController* Controller)
{
	if (!Controller || !GetWorld())
	{
		return;
	}

	AActor* Start = FindPlayerStart(Controller);
	if (!Start)
	{
		TArray<AActor*> Starts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), Starts);
		if (Starts.Num() > 0)
		{
			Start = Starts[FMath::RandRange(0, Starts.Num() - 1)];
		}
	}

	if (!Start)
	{
		return;
	}

	if (APawn* Pawn = Controller->GetPawn())
	{
		Pawn->SetActorLocationAndRotation(Start->GetActorLocation(), Start->GetActorRotation(), false, nullptr, ETeleportType::TeleportPhysics);
		if (APlayerController* PC = Cast<APlayerController>(Controller))
		{
			Pawn->EnableInput(PC);
			PC->SetControlRotation(Start->GetActorRotation());
		}
	}
}

void AFallingFloorGameMode::CheckWinCondition()
{
	AFallingFloorGameState* GS = GetGameState<AFallingFloorGameState>();
	if (!GS || !HasAuthority() || !GS->IsMatchInProgress() || bReturningToHub)
	{
		return;
	}

	const int32 Remaining = GS->GetPlayersRemaining();
	const int32 Initial = GS->GetInitialPlayerCount();

	// Solo / test session: end as soon as the only player falls.
	if (Initial <= 1)
	{
		if (Remaining <= 0)
		{
			HandleMatchEnded();
		}
		return;
	}

	if (Remaining > 1)
	{
		return;
	}

	for (APlayerState* BasePS : GS->PlayerArray)
	{
		AFallingFloorPlayerState* PS = Cast<AFallingFloorPlayerState>(BasePS);
		if (!PS || PS->IsEliminated())
		{
			continue;
		}

		if (AFallingFloorPlayerController* PC = Cast<AFallingFloorPlayerController>(PS->GetPlayerController()))
		{
			PC->ClientShowVictory();
			PC->ClientUpdatePlayersRemaining(Remaining);
		}
	}

	HandleMatchEnded();
}

void AFallingFloorGameMode::HandleMatchEnded()
{
	AFallingFloorGameState* GS = GetGameState<AFallingFloorGameState>();
	if (!GS || !HasAuthority() || bReturningToHub)
	{
		return;
	}

	bReturningToHub = true;
	GS->SetMatchPhase(EFallingFloorMatchPhase::Ended);
	GetWorldTimerManager().ClearTimer(FallPollTimerHandle);
	GetWorldTimerManager().ClearTimer(CountdownTimerHandle);

	UE_LOG(LogIpvMulti, Warning, TEXT("FallingFloor match ended. Returning to hub in %.1fs"), ReturnToHubDelay);

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (AFallingFloorPlayerController* PC = Cast<AFallingFloorPlayerController>(It->Get()))
		{
			PC->ClientNotifyMatchPhase(EFallingFloorMatchPhase::Ended);

			if (APawn* Pawn = PC->GetPawn())
			{
				Pawn->DisableInput(PC);
			}
		}
	}

	GetWorldTimerManager().ClearTimer(ReturnToHubTimerHandle);
	GetWorldTimerManager().SetTimer(
		ReturnToHubTimerHandle,
		this,
		&AFallingFloorGameMode::ReturnToHub,
		ReturnToHubDelay,
		false);
}

void AFallingFloorGameMode::ReturnToHub()
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

	// Same pattern used by OpenLobby in this project.
	if (!TravelURL.Contains(TEXT("listen"), ESearchCase::IgnoreCase))
	{
		TravelURL += TravelURL.Contains(TEXT("?")) ? TEXT("&listen") : TEXT("?listen");
	}

	UE_LOG(LogIpvMulti, Warning, TEXT("FallingFloor ServerTravel -> %s"), *TravelURL);

	// Match OpenLobby: relative ServerTravel with ?listen (absolute=true can fail in PIE).
	World->ServerTravel(TravelURL, false);
}

void AFallingFloorGameMode::SwitchToSpectatorView(APlayerController* PC)
{
	if (!PC || !SpectatorViewClass)
	{
		return;
	}

	TArray<AActor*> Views;
	UGameplayStatics::GetAllActorsOfClass(this, SpectatorViewClass, Views);
	if (Views.Num() == 0)
	{
		return;
	}

	PC->SetViewTargetWithBlend(Views[0], SpectatorBlendTime, VTBlend_Cubic);
}

int32 AFallingFloorGameMode::CountActivePlayers() const
{
	int32 Count = 0;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC)
		{
			continue;
		}

		const AFallingFloorPlayerState* PS = PC->GetPlayerState<AFallingFloorPlayerState>();
		if (PS && !PS->IsEliminated() && PC->GetPawn() != nullptr)
		{
			++Count;
		}
	}
	return Count;
}
