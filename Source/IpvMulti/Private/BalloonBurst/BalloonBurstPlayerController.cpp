// Copyright © 2026 IpvMulti

#include "BalloonBurst/BalloonBurstPlayerController.h"

#include "BalloonBurst/BalloonBurstBalloon.h"
#include "BalloonBurst/BalloonBurstGameMode.h"
#include "BalloonBurst/BalloonBurstGameState.h"
#include "BalloonBurst/BalloonBurstPlayerState.h"
#include "BalloonBurst/BalloonBurstUI.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Engine/LocalPlayer.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

ABalloonBurstPlayerController::ABalloonBurstPlayerController()
{
	BalloonBurstUIClass = UBalloonBurstUI::StaticClass();

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

	static ConstructorHelpers::FObjectFinder<UInputAction> JumpIA(
		TEXT("/Game/Input/Actions/IA_Jump.IA_Jump"));
	if (JumpIA.Succeeded())
	{
		PumpAction = JumpIA.Object;
	}
}

void ABalloonBurstPlayerController::EnsureDefaultMappingContexts()
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

void ABalloonBurstPlayerController::EnsurePumpAction()
{
	if (!PumpAction)
	{
		PumpAction = LoadObject<UInputAction>(nullptr, TEXT("/Game/Input/Actions/IA_Jump.IA_Jump"));
	}
}

void ABalloonBurstPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalPlayerController())
	{
		EnsureDefaultMappingContexts();
		EnsurePumpAction();
		ApplyGameplayInputMode();
		CreateHUD();
		BindGameStateEvents();
		StripLegacyPlayerHUD();

		GetWorldTimerManager().SetTimer(
			LegacyHudStripTimerHandle,
			this,
			&ABalloonBurstPlayerController::StripLegacyPlayerHUD,
			0.25f,
			true);
	}
}

void ABalloonBurstPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	ConfigurePawnForMinigame(InPawn);

	if (IsLocalPlayerController())
	{
		ApplyGameplayInputMode();
	}
}

void ABalloonBurstPlayerController::ConfigurePawnForMinigame(APawn* InPawn)
{
	if (!InPawn)
	{
		return;
	}

	// Keep looking around, but lock locomotion so stations stay tidy.
	if (ACharacter* MinigameCharacter = Cast<ACharacter>(InPawn))
	{
		MinigameCharacter->JumpMaxCount = 0;
		if (UCharacterMovementComponent* Move = MinigameCharacter->GetCharacterMovement())
		{
			Move->StopMovementImmediately();
			Move->DisableMovement();
		}
	}

	InPawn->EnableInput(this);
}

void ABalloonBurstPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(LegacyHudStripTimerHandle);

	if (ABalloonBurstGameState* GS = GetWorld() ? GetWorld()->GetGameState<ABalloonBurstGameState>() : nullptr)
	{
		GS->OnPlayerCountChanged.RemoveDynamic(this, &ABalloonBurstPlayerController::HandlePlayerCountChanged);
		GS->OnMatchPhaseChanged.RemoveDynamic(this, &ABalloonBurstPlayerController::HandleMatchPhaseChanged);
		GS->OnCountdownChanged.RemoveDynamic(this, &ABalloonBurstPlayerController::HandleCountdownChanged);
	}

	Super::EndPlay(EndPlayReason);
}

void ABalloonBurstPlayerController::ApplyGameplayInputMode()
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	bShowMouseCursor = false;
}

void ABalloonBurstPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (!IsLocalPlayerController())
	{
		return;
	}

	EnsureDefaultMappingContexts();
	EnsurePumpAction();

	if (!bMappingContextsAdded)
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
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

	if (!bPumpBound && PumpAction)
	{
		if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
		{
			EIC->BindAction(PumpAction, ETriggerEvent::Started, this, &ABalloonBurstPlayerController::HandlePumpPressed);
			bPumpBound = true;
		}
	}
}

void ABalloonBurstPlayerController::SetOwnedBalloon(ABalloonBurstBalloon* InBalloon)
{
	OwnedBalloon = InBalloon;
}

void ABalloonBurstPlayerController::HandlePumpPressed()
{
	if (bResultScreenShown)
	{
		return;
	}

	const ABalloonBurstGameState* GS = GetWorld() ? GetWorld()->GetGameState<ABalloonBurstGameState>() : nullptr;
	if (!GS || !GS->IsMatchInProgress())
	{
		return;
	}

	ServerRequestPump();
}

void ABalloonBurstPlayerController::ServerRequestPump_Implementation()
{
	if (bResultScreenShown)
	{
		return;
	}

	ABalloonBurstGameState* GS = GetWorld() ? GetWorld()->GetGameState<ABalloonBurstGameState>() : nullptr;
	if (!GS || !GS->IsMatchInProgress())
	{
		return;
	}

	ABalloonBurstPlayerState* PS = GetPlayerState<ABalloonBurstPlayerState>();
	if (!PS || PS->HasBurstBalloon())
	{
		return;
	}

	ABalloonBurstBalloon* Balloon = OwnedBalloon.Get();
	if (!Balloon)
	{
		if (ABalloonBurstGameMode* GM = GetWorld()->GetAuthGameMode<ABalloonBurstGameMode>())
		{
			Balloon = GM->FindBalloonForController(this);
			OwnedBalloon = Balloon;
		}
	}

	if (!Balloon)
	{
		return;
	}

	const bool bBurst = Balloon->ServerAddPump();
	ClientUpdatePumpProgress(Balloon->GetCurrentPumps(), Balloon->GetPumpsRequired());

	if (bBurst)
	{
		if (ABalloonBurstGameMode* GM = GetWorld()->GetAuthGameMode<ABalloonBurstGameMode>())
		{
			GM->NotifyBalloonBurst(this);
		}
	}
}

void ABalloonBurstPlayerController::CreateHUD()
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	StripLegacyPlayerHUD();

	if (!BalloonBurstUIClass || BalloonBurstUIInstance)
	{
		return;
	}

	BalloonBurstUIInstance = CreateWidget<UBalloonBurstUI>(this, BalloonBurstUIClass);
	if (BalloonBurstUIInstance)
	{
		BalloonBurstUIInstance->SetIsFocusable(false);
		BalloonBurstUIInstance->SetVisibility(ESlateVisibility::HitTestInvisible);
		BalloonBurstUIInstance->AddToViewport(10);
		ApplyGameplayInputMode();

		if (const ABalloonBurstGameState* GS = GetWorld()->GetGameState<ABalloonBurstGameState>())
		{
			BalloonBurstUIInstance->UpdatePlayerCount(GS->GetPlayerCount());
			BalloonBurstUIInstance->NotifyMatchPhase(GS->GetMatchPhase());
			BalloonBurstUIInstance->UpdateCountdown(GS->GetCountdownSeconds());
			BalloonBurstUIInstance->UpdatePumpProgress(0, GS->GetPumpsRequired());
		}
	}
}

void ABalloonBurstPlayerController::StripLegacyPlayerHUD()
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	TArray<UUserWidget*> Widgets;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(this, Widgets, UUserWidget::StaticClass(), false);
	for (UUserWidget* Widget : Widgets)
	{
		if (!Widget || Widget == BalloonBurstUIInstance)
		{
			continue;
		}

		if (Widget->IsA<UBalloonBurstUI>())
		{
			continue;
		}

		if (Widget->GetOwningPlayer() == this && Widget->IsInViewport())
		{
			Widget->RemoveFromParent();
		}
	}

	ApplyGameplayInputMode();

	++LegacyHudStripCount;
	if (BalloonBurstUIInstance && LegacyHudStripCount >= 8)
	{
		GetWorldTimerManager().ClearTimer(LegacyHudStripTimerHandle);
	}
}

void ABalloonBurstPlayerController::BindGameStateEvents()
{
	ABalloonBurstGameState* GS = GetWorld() ? GetWorld()->GetGameState<ABalloonBurstGameState>() : nullptr;
	if (!GS)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimerForNextTick(
				FTimerDelegate::CreateUObject(this, &ABalloonBurstPlayerController::BindGameStateEvents));
		}
		return;
	}

	GS->OnPlayerCountChanged.AddUniqueDynamic(this, &ABalloonBurstPlayerController::HandlePlayerCountChanged);
	GS->OnMatchPhaseChanged.AddUniqueDynamic(this, &ABalloonBurstPlayerController::HandleMatchPhaseChanged);
	GS->OnCountdownChanged.AddUniqueDynamic(this, &ABalloonBurstPlayerController::HandleCountdownChanged);

	HandlePlayerCountChanged(GS->GetPlayerCount());
	HandleMatchPhaseChanged(GS->GetMatchPhase());
	HandleCountdownChanged(GS->GetCountdownSeconds());
}

void ABalloonBurstPlayerController::HandlePlayerCountChanged(int32 PlayerCount)
{
	if (BalloonBurstUIInstance)
	{
		BalloonBurstUIInstance->UpdatePlayerCount(PlayerCount);
	}
}

void ABalloonBurstPlayerController::HandleMatchPhaseChanged(EBalloonBurstMatchPhase NewPhase)
{
	if (BalloonBurstUIInstance)
	{
		BalloonBurstUIInstance->NotifyMatchPhase(NewPhase);
	}
}

void ABalloonBurstPlayerController::HandleCountdownChanged(int32 SecondsRemaining)
{
	if (BalloonBurstUIInstance)
	{
		BalloonBurstUIInstance->UpdateCountdown(SecondsRemaining);
	}
}

void ABalloonBurstPlayerController::ClientMatchStarted_Implementation(int32 PlayerCount)
{
	CreateHUD();
	BindGameStateEvents();
	ApplyGameplayInputMode();
	bResultScreenShown = false;
	ConfigurePawnForMinigame(GetPawn());

	if (BalloonBurstUIInstance)
	{
		BalloonBurstUIInstance->OnMatchStarted(PlayerCount);
	}
}

void ABalloonBurstPlayerController::ClientUpdatePlayerCount_Implementation(int32 PlayerCount)
{
	if (BalloonBurstUIInstance)
	{
		BalloonBurstUIInstance->UpdatePlayerCount(PlayerCount);
	}
}

void ABalloonBurstPlayerController::ClientUpdateCountdown_Implementation(int32 SecondsRemaining)
{
	CreateHUD();
	if (BalloonBurstUIInstance)
	{
		BalloonBurstUIInstance->UpdateCountdown(SecondsRemaining);
	}
}

void ABalloonBurstPlayerController::ClientNotifyMatchPhase_Implementation(EBalloonBurstMatchPhase NewPhase)
{
	if (BalloonBurstUIInstance)
	{
		BalloonBurstUIInstance->NotifyMatchPhase(NewPhase);
	}
}

void ABalloonBurstPlayerController::ClientUpdatePumpProgress_Implementation(int32 CurrentPumps, int32 PumpsRequired)
{
	CreateHUD();
	if (BalloonBurstUIInstance)
	{
		BalloonBurstUIInstance->UpdatePumpProgress(CurrentPumps, PumpsRequired);
	}
}

void ABalloonBurstPlayerController::ClientShowVictory_Implementation()
{
	if (bResultScreenShown)
	{
		return;
	}
	bResultScreenShown = true;

	CreateHUD();
	if (BalloonBurstUIInstance)
	{
		BalloonBurstUIInstance->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		BalloonBurstUIInstance->ShowVictory();
	}
}

void ABalloonBurstPlayerController::ClientShowDefeat_Implementation()
{
	if (bResultScreenShown)
	{
		return;
	}
	bResultScreenShown = true;

	CreateHUD();
	if (BalloonBurstUIInstance)
	{
		BalloonBurstUIInstance->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		BalloonBurstUIInstance->ShowDefeat();
	}
}
