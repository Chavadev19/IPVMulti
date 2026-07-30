// Copyright © 2026 IpvMulti

#include "FallingFloor/FallingFloorPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "FallingFloor/FallingFloorGameState.h"
#include "FallingFloor/FallingFloorUI.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

AFallingFloorPlayerController::AFallingFloorPlayerController()
{
	FallingFloorUIClass = UFallingFloorUI::StaticClass();

	static ConstructorHelpers::FObjectFinder<UInputMappingContext> DefaultIMC(
		TEXT("/Game/Input/IMC_Default.IMC_Default"));
	if (DefaultIMC.Succeeded())
	{
		DefaultMappingContexts.Add(DefaultIMC.Object);
	}

	static ConstructorHelpers::FObjectFinder<UInputMappingContext> MouseIMC(
		TEXT("/Game/Input/IMC_MouseLook.IMC_MouseLook"));
	if (MouseIMC.Succeeded())
	{
		MobileExcludedMappingContexts.Add(MouseIMC.Object);
	}
}

void AFallingFloorPlayerController::EnsureDefaultMappingContexts()
{
	if (DefaultMappingContexts.Num() == 0)
	{
		if (UInputMappingContext* Loaded = LoadObject<UInputMappingContext>(
			nullptr, TEXT("/Game/Input/IMC_Default.IMC_Default")))
		{
			DefaultMappingContexts.Add(Loaded);
		}
	}

	if (MobileExcludedMappingContexts.Num() == 0)
	{
		if (UInputMappingContext* Loaded = LoadObject<UInputMappingContext>(
			nullptr, TEXT("/Game/Input/IMC_MouseLook.IMC_MouseLook")))
		{
			MobileExcludedMappingContexts.Add(Loaded);
		}
	}
}

void AFallingFloorPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalPlayerController())
	{
		EnsureDefaultMappingContexts();
		ApplyGameplayInputMode();
		CreateHUD();
		BindGameStateEvents();
		StripLegacyPlayerHUD();

		// Character BeginPlay may create health/objective widgets slightly later.
		GetWorldTimerManager().SetTimer(
			LegacyHudStripTimerHandle,
			this,
			&AFallingFloorPlayerController::StripLegacyPlayerHUD,
			0.25f,
			true);
	}
}

void AFallingFloorPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// Warmup and match both allow movement until elimination.
	if (InPawn)
	{
		InPawn->EnableInput(this);
	}

	if (IsLocalPlayerController())
	{
		ApplyGameplayInputMode();
	}
}

void AFallingFloorPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(LegacyHudStripTimerHandle);

	if (AFallingFloorGameState* GS = GetWorld() ? GetWorld()->GetGameState<AFallingFloorGameState>() : nullptr)
	{
		GS->OnPlayersRemainingChanged.RemoveDynamic(this, &AFallingFloorPlayerController::HandlePlayersRemainingChanged);
		GS->OnMatchPhaseChanged.RemoveDynamic(this, &AFallingFloorPlayerController::HandleMatchPhaseChanged);
		GS->OnCountdownChanged.RemoveDynamic(this, &AFallingFloorPlayerController::HandleCountdownChanged);
	}

	Super::EndPlay(EndPlayReason);
}

void AFallingFloorPlayerController::ApplyGameplayInputMode()
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	bShowMouseCursor = false;
}

void AFallingFloorPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (!IsLocalPlayerController() || bMappingContextsAdded)
	{
		return;
	}

	EnsureDefaultMappingContexts();

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		for (UInputMappingContext* Context : DefaultMappingContexts)
		{
			if (Context)
			{
				Subsystem->AddMappingContext(Context, 0);
			}
		}

		for (UInputMappingContext* Context : MobileExcludedMappingContexts)
		{
			if (Context)
			{
				Subsystem->AddMappingContext(Context, 0);
			}
		}

		bMappingContextsAdded = DefaultMappingContexts.Num() > 0 || MobileExcludedMappingContexts.Num() > 0;
	}
}

void AFallingFloorPlayerController::CreateHUD()
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	StripLegacyPlayerHUD();

	if (!FallingFloorUIClass || FallingFloorUIInstance)
	{
		return;
	}

	FallingFloorUIInstance = CreateWidget<UFallingFloorUI>(this, FallingFloorUIClass);
	if (FallingFloorUIInstance)
	{
		// Never steal mouse/keyboard from gameplay.
		FallingFloorUIInstance->SetIsFocusable(false);
		FallingFloorUIInstance->SetVisibility(ESlateVisibility::HitTestInvisible);
		FallingFloorUIInstance->AddToViewport(10);
		ApplyGameplayInputMode();

		if (const AFallingFloorGameState* GS = GetWorld()->GetGameState<AFallingFloorGameState>())
		{
			FallingFloorUIInstance->UpdatePlayersRemaining(GS->GetPlayersRemaining());
			FallingFloorUIInstance->NotifyMatchPhase(GS->GetMatchPhase());
			FallingFloorUIInstance->UpdateCountdown(GS->GetCountdownSeconds());
		}
	}
}

void AFallingFloorPlayerController::StripLegacyPlayerHUD()
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	TArray<UUserWidget*> Widgets;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(this, Widgets, UUserWidget::StaticClass(), false);
	for (UUserWidget* Widget : Widgets)
	{
		if (!Widget || Widget == FallingFloorUIInstance)
		{
			continue;
		}

		if (Widget->IsA<UFallingFloorUI>())
		{
			continue;
		}

		if (Widget->GetOwningPlayer() == this && Widget->IsInViewport())
		{
			Widget->RemoveFromParent();
		}
	}

	ApplyGameplayInputMode();

	// Stop the recurring strip once our HUD exists and a couple of sweeps have run.
	++LegacyHudStripCount;
	if (FallingFloorUIInstance && LegacyHudStripCount >= 8)
	{
		GetWorldTimerManager().ClearTimer(LegacyHudStripTimerHandle);
	}
}

void AFallingFloorPlayerController::BindGameStateEvents()
{
	AFallingFloorGameState* GS = GetWorld() ? GetWorld()->GetGameState<AFallingFloorGameState>() : nullptr;
	if (!GS)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &AFallingFloorPlayerController::BindGameStateEvents));
		}
		return;
	}

	GS->OnPlayersRemainingChanged.AddUniqueDynamic(this, &AFallingFloorPlayerController::HandlePlayersRemainingChanged);
	GS->OnMatchPhaseChanged.AddUniqueDynamic(this, &AFallingFloorPlayerController::HandleMatchPhaseChanged);
	GS->OnCountdownChanged.AddUniqueDynamic(this, &AFallingFloorPlayerController::HandleCountdownChanged);

	HandlePlayersRemainingChanged(GS->GetPlayersRemaining());
	HandleMatchPhaseChanged(GS->GetMatchPhase());
	HandleCountdownChanged(GS->GetCountdownSeconds());
}

void AFallingFloorPlayerController::HandlePlayersRemainingChanged(int32 PlayersRemaining)
{
	if (FallingFloorUIInstance)
	{
		FallingFloorUIInstance->UpdatePlayersRemaining(PlayersRemaining);
	}
}

void AFallingFloorPlayerController::HandleMatchPhaseChanged(EFallingFloorMatchPhase NewPhase)
{
	if (FallingFloorUIInstance)
	{
		FallingFloorUIInstance->NotifyMatchPhase(NewPhase);
	}
}

void AFallingFloorPlayerController::HandleCountdownChanged(int32 SecondsRemaining)
{
	if (FallingFloorUIInstance)
	{
		FallingFloorUIInstance->UpdateCountdown(SecondsRemaining);
	}
}

void AFallingFloorPlayerController::ClientMatchStarted_Implementation(int32 PlayersAlive)
{
	CreateHUD();
	BindGameStateEvents();
	ApplyGameplayInputMode();
	bResultScreenShown = false;

	if (APawn* ControlledPawn = GetPawn())
	{
		ControlledPawn->EnableInput(this);
	}

	if (FallingFloorUIInstance)
	{
		FallingFloorUIInstance->UpdateCountdown(0);
		FallingFloorUIInstance->UpdatePlayersRemaining(PlayersAlive);
		FallingFloorUIInstance->NotifyMatchPhase(EFallingFloorMatchPhase::InProgress);
		FallingFloorUIInstance->OnMatchStarted(PlayersAlive);
	}
}

void AFallingFloorPlayerController::ClientUpdatePlayersRemaining_Implementation(int32 PlayersAlive)
{
	if (FallingFloorUIInstance)
	{
		FallingFloorUIInstance->UpdatePlayersRemaining(PlayersAlive);
	}
}

void AFallingFloorPlayerController::ClientUpdateCountdown_Implementation(int32 SecondsRemaining)
{
	CreateHUD();
	if (FallingFloorUIInstance)
	{
		FallingFloorUIInstance->UpdateCountdown(SecondsRemaining);
	}
}

void AFallingFloorPlayerController::ClientNotifyMatchPhase_Implementation(EFallingFloorMatchPhase NewPhase)
{
	if (FallingFloorUIInstance)
	{
		FallingFloorUIInstance->NotifyMatchPhase(NewPhase);
	}
}

void AFallingFloorPlayerController::ClientShowVictory_Implementation()
{
	if (bResultScreenShown)
	{
		return;
	}
	bResultScreenShown = true;

	CreateHUD();
	if (FallingFloorUIInstance)
	{
		FallingFloorUIInstance->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		FallingFloorUIInstance->ShowVictory();
	}
}

void AFallingFloorPlayerController::ClientShowDefeat_Implementation()
{
	if (bResultScreenShown)
	{
		return;
	}
	bResultScreenShown = true;

	CreateHUD();
	if (FallingFloorUIInstance)
	{
		FallingFloorUIInstance->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		FallingFloorUIInstance->ShowDefeat();
	}
}
