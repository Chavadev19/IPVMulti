// Copyright © 2026 IpvMulti

#include "BalloonBurst/BalloonBurstUI.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "TimerManager.h"

void UBalloonBurstUI::NativeConstruct()
{
	Super::NativeConstruct();

	SetIsFocusable(false);
	SetVisibility(ESlateVisibility::HitTestInvisible);

	if (UBorder* Victory = GetVictory())
	{
		Victory->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (UBorder* Defeat = GetDefeat())
	{
		Defeat->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (UBorder* Draw = GetDraw())
	{
		Draw->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (UTextBlock* Instructions = GetInstructions())
	{
		Instructions->SetText(FText::FromString(
			TEXT("¡Balloon Burst!\nPulsa ESPACIO repetidamente para inflar tu globo.\nEl primero en reventarlo gana — ¡cuidado con los empates!")));
	}
	if (UTextBlock* Status = GetStatus())
	{
		Status->SetText(FText::FromString(TEXT("Prepárate — la partida empieza pronto")));
	}
	if (UTextBlock* Countdown = GetCountdown())
	{
		Countdown->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	if (UTextBlock* Progress = GetProgress())
	{
		Progress->SetText(FText::FromString(TEXT("Inflado: 0")));
	}
}

TSharedRef<SWidget> UBalloonBurstUI::RebuildWidget()
{
	if (WidgetTree && WidgetTree->RootWidget)
	{
		return Super::RebuildWidget();
	}

	if (!WidgetTree)
	{
		return Super::RebuildWidget();
	}

	bBuiltRuntimeLayout = true;

	UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
	WidgetTree->RootWidget = Root;

	RuntimePlayersText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("PlayersText"));
	RuntimePlayersText->SetText(FText::FromString(TEXT("Jugadores: -")));
	FSlateFontInfo PlayersFont = RuntimePlayersText->GetFont();
	PlayersFont.Size = 32;
	RuntimePlayersText->SetFont(PlayersFont);
	RuntimePlayersText->SetColorAndOpacity(FLinearColor::White);
	if (UCanvasPanelSlot* PlayersSlot = Root->AddChildToCanvas(RuntimePlayersText))
	{
		PlayersSlot->SetAnchors(FAnchors(0.5f, 0.0f));
		PlayersSlot->SetAlignment(FVector2D(0.5f, 0.0f));
		PlayersSlot->SetPosition(FVector2D(0.0f, 24.0f));
		PlayersSlot->SetAutoSize(true);
	}

	RuntimeInstructionsText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("InstructionsText"));
	RuntimeInstructionsText->SetText(FText::FromString(
		TEXT("¡Balloon Burst!\nPulsa ESPACIO repetidamente para inflar tu globo.\nEl primero en reventarlo gana — ¡cuidado con los empates!")));
	FSlateFontInfo InstrFont = RuntimeInstructionsText->GetFont();
	InstrFont.Size = 20;
	RuntimeInstructionsText->SetFont(InstrFont);
	RuntimeInstructionsText->SetJustification(ETextJustify::Center);
	RuntimeInstructionsText->SetColorAndOpacity(FLinearColor(0.95f, 0.95f, 0.85f));
	if (UCanvasPanelSlot* InstrSlot = Root->AddChildToCanvas(RuntimeInstructionsText))
	{
		InstrSlot->SetAnchors(FAnchors(0.5f, 0.0f));
		InstrSlot->SetAlignment(FVector2D(0.5f, 0.0f));
		InstrSlot->SetPosition(FVector2D(0.0f, 70.0f));
		InstrSlot->SetAutoSize(true);
	}

	RuntimeStatusText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StatusText"));
	RuntimeStatusText->SetText(FText::FromString(TEXT("Prepárate — la partida empieza pronto")));
	FSlateFontInfo StatusFont = RuntimeStatusText->GetFont();
	StatusFont.Size = 22;
	RuntimeStatusText->SetFont(StatusFont);
	RuntimeStatusText->SetColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f));
	if (UCanvasPanelSlot* StatusSlot = Root->AddChildToCanvas(RuntimeStatusText))
	{
		StatusSlot->SetAnchors(FAnchors(0.5f, 0.0f));
		StatusSlot->SetAlignment(FVector2D(0.5f, 0.0f));
		StatusSlot->SetPosition(FVector2D(0.0f, 160.0f));
		StatusSlot->SetAutoSize(true);
	}

	RuntimeCountdownText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("CountdownText"));
	RuntimeCountdownText->SetText(FText::FromString(TEXT("5")));
	FSlateFontInfo CountdownFont = RuntimeCountdownText->GetFont();
	CountdownFont.Size = 110;
	RuntimeCountdownText->SetFont(CountdownFont);
	RuntimeCountdownText->SetJustification(ETextJustify::Center);
	RuntimeCountdownText->SetColorAndOpacity(FLinearColor(1.0f, 0.9f, 0.15f));
	if (UCanvasPanelSlot* CountdownSlot = Root->AddChildToCanvas(RuntimeCountdownText))
	{
		CountdownSlot->SetAnchors(FAnchors(0.5f, 0.42f));
		CountdownSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		CountdownSlot->SetPosition(FVector2D(0.0f, 0.0f));
		CountdownSlot->SetAutoSize(true);
	}

	RuntimeProgressText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ProgressText"));
	RuntimeProgressText->SetText(FText::FromString(TEXT("Inflado: 0 / 20")));
	FSlateFontInfo ProgressFont = RuntimeProgressText->GetFont();
	ProgressFont.Size = 28;
	RuntimeProgressText->SetFont(ProgressFont);
	RuntimeProgressText->SetColorAndOpacity(FLinearColor(0.55f, 0.85f, 1.0f));
	if (UCanvasPanelSlot* ProgressSlot = Root->AddChildToCanvas(RuntimeProgressText))
	{
		ProgressSlot->SetAnchors(FAnchors(0.5f, 1.0f));
		ProgressSlot->SetAlignment(FVector2D(0.5f, 1.0f));
		ProgressSlot->SetPosition(FVector2D(0.0f, -48.0f));
		ProgressSlot->SetAutoSize(true);
	}

	RuntimeVictoryBanner = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("VictoryBanner"));
	RuntimeVictoryBanner->SetBrushColor(FLinearColor(0.05f, 0.45f, 0.15f, 0.9f));
	RuntimeVictoryBanner->SetVisibility(ESlateVisibility::Collapsed);
	UTextBlock* VictoryText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("VictoryText"));
	VictoryText->SetText(FText::FromString(TEXT("YOU WIN")));
	FSlateFontInfo BannerFont = VictoryText->GetFont();
	BannerFont.Size = 72;
	VictoryText->SetFont(BannerFont);
	VictoryText->SetJustification(ETextJustify::Center);
	VictoryText->SetColorAndOpacity(FLinearColor::White);
	RuntimeVictoryBanner->SetContent(VictoryText);
	if (UCanvasPanelSlot* VictorySlot = Root->AddChildToCanvas(RuntimeVictoryBanner))
	{
		VictorySlot->SetAnchors(FAnchors(0.5f, 0.5f));
		VictorySlot->SetAlignment(FVector2D(0.5f, 0.5f));
		VictorySlot->SetPosition(FVector2D(0.0f, 40.0f));
		VictorySlot->SetSize(FVector2D(560.0f, 150.0f));
	}

	RuntimeDefeatBanner = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("DefeatBanner"));
	RuntimeDefeatBanner->SetBrushColor(FLinearColor(0.45f, 0.05f, 0.08f, 0.9f));
	RuntimeDefeatBanner->SetVisibility(ESlateVisibility::Collapsed);
	UTextBlock* DefeatText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("DefeatText"));
	DefeatText->SetText(FText::FromString(TEXT("YOU LOSE")));
	DefeatText->SetFont(BannerFont);
	DefeatText->SetJustification(ETextJustify::Center);
	DefeatText->SetColorAndOpacity(FLinearColor::White);
	RuntimeDefeatBanner->SetContent(DefeatText);
	if (UCanvasPanelSlot* DefeatSlot = Root->AddChildToCanvas(RuntimeDefeatBanner))
	{
		DefeatSlot->SetAnchors(FAnchors(0.5f, 0.5f));
		DefeatSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		DefeatSlot->SetPosition(FVector2D(0.0f, 40.0f));
		DefeatSlot->SetSize(FVector2D(560.0f, 150.0f));
	}

	RuntimeDrawBanner = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("DrawBanner"));
	RuntimeDrawBanner->SetBrushColor(FLinearColor(0.15f, 0.25f, 0.45f, 0.9f));
	RuntimeDrawBanner->SetVisibility(ESlateVisibility::Collapsed);
	UTextBlock* DrawText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("DrawText"));
	DrawText->SetText(FText::FromString(TEXT("DRAW")));
	DrawText->SetFont(BannerFont);
	DrawText->SetJustification(ETextJustify::Center);
	DrawText->SetColorAndOpacity(FLinearColor::White);
	RuntimeDrawBanner->SetContent(DrawText);
	if (UCanvasPanelSlot* DrawSlot = Root->AddChildToCanvas(RuntimeDrawBanner))
	{
		DrawSlot->SetAnchors(FAnchors(0.5f, 0.5f));
		DrawSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		DrawSlot->SetPosition(FVector2D(0.0f, 40.0f));
		DrawSlot->SetSize(FVector2D(560.0f, 150.0f));
	}

	return Super::RebuildWidget();
}

void UBalloonBurstUI::UpdatePlayerCount(int32 PlayerCount)
{
	if (UTextBlock* Text = GetPlayers())
	{
		Text->SetText(FText::FromString(FString::Printf(TEXT("Jugadores: %d"), PlayerCount)));
	}
	BP_UpdatePlayerCount(PlayerCount);
}

void UBalloonBurstUI::UpdateCountdown(int32 SecondsRemaining)
{
	if (bShowingStart)
	{
		BP_UpdateCountdown(SecondsRemaining);
		return;
	}

	if (UTextBlock* Countdown = GetCountdown())
	{
		if (SecondsRemaining > 0)
		{
			Countdown->SetVisibility(ESlateVisibility::HitTestInvisible);
			Countdown->SetText(FText::AsNumber(SecondsRemaining));
			Countdown->SetColorAndOpacity(FLinearColor(1.0f, 0.9f, 0.15f));
		}
		else
		{
			Countdown->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	if (UTextBlock* Status = GetStatus())
	{
		if (SecondsRemaining > 0)
		{
			Status->SetText(FText::FromString(
				FString::Printf(TEXT("Empieza en %d..."), SecondsRemaining)));
		}
	}

	BP_UpdateCountdown(SecondsRemaining);
}

void UBalloonBurstUI::NotifyMatchPhase(EBalloonBurstMatchPhase NewPhase)
{
	if (UTextBlock* Status = GetStatus())
	{
		switch (NewPhase)
		{
		case EBalloonBurstMatchPhase::WaitingToStart:
			Status->SetText(FText::FromString(TEXT("Prepárate — la partida empieza pronto")));
			if (UTextBlock* Instructions = GetInstructions())
			{
				Instructions->SetVisibility(ESlateVisibility::HitTestInvisible);
			}
			break;
		case EBalloonBurstMatchPhase::InProgress:
			Status->SetText(FText::FromString(TEXT("¡Infla tu globo! (ESPACIO)")));
			if (UTextBlock* Instructions = GetInstructions())
			{
				Instructions->SetVisibility(ESlateVisibility::Collapsed);
			}
			break;
		case EBalloonBurstMatchPhase::Ended:
			Status->SetText(FText::FromString(TEXT("Volviendo al hub...")));
			HideStartBanner();
			break;
		}
	}
	BP_NotifyMatchPhase(NewPhase);
}

void UBalloonBurstUI::OnMatchStarted(int32 PlayerCount)
{
	UpdatePlayerCount(PlayerCount);
	ShowStartBanner();
	NotifyMatchPhase(EBalloonBurstMatchPhase::InProgress);
	BP_OnMatchStarted(PlayerCount);
}

void UBalloonBurstUI::ShowStartBanner()
{
	bShowingStart = true;

	if (UTextBlock* Countdown = GetCountdown())
	{
		Countdown->SetVisibility(ESlateVisibility::HitTestInvisible);
		Countdown->SetText(FText::FromString(TEXT("START!")));
		Countdown->SetColorAndOpacity(FLinearColor(0.2f, 1.0f, 0.35f));
	}

	if (UTextBlock* Status = GetStatus())
	{
		Status->SetText(FText::FromString(TEXT("¡YA! Infla tu globo")));
	}

	BP_ShowStartBanner();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(StartBannerTimerHandle);
		World->GetTimerManager().SetTimer(
			StartBannerTimerHandle,
			this,
			&UBalloonBurstUI::HideStartBanner,
			1.25f,
			false);
	}
}

void UBalloonBurstUI::HideStartBanner()
{
	bShowingStart = false;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(StartBannerTimerHandle);
	}

	if (UTextBlock* Countdown = GetCountdown())
	{
		Countdown->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UBalloonBurstUI::UpdatePumpProgress(int32 CurrentPumps, int32 PumpsRequired)
{
	if (UTextBlock* Progress = GetProgress())
	{
		const int32 SafeRequired = FMath::Max(1, PumpsRequired);
		const int32 Pct = FMath::Clamp(
			FMath::RoundToInt(100.0f * static_cast<float>(CurrentPumps) / static_cast<float>(SafeRequired)),
			0,
			100);
		Progress->SetText(FText::FromString(
			FString::Printf(TEXT("Inflado: %d / %d  (%d%%)"), CurrentPumps, SafeRequired, Pct)));
	}
	BP_UpdatePumpProgress(CurrentPumps, PumpsRequired);
}

void UBalloonBurstUI::ShowVictory()
{
	HideStartBanner();

	if (UBorder* Defeat = GetDefeat())
	{
		Defeat->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (UBorder* Draw = GetDraw())
	{
		Draw->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (UBorder* Victory = GetVictory())
	{
		Victory->SetVisibility(ESlateVisibility::Visible);
	}
	if (UTextBlock* Status = GetStatus())
	{
		Status->SetText(FText::FromString(TEXT("¡Reventaste el globo! Volviendo al hub...")));
	}
	BP_ShowVictory();
}

void UBalloonBurstUI::ShowDefeat()
{
	HideStartBanner();

	if (UBorder* Victory = GetVictory())
	{
		Victory->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (UBorder* Draw = GetDraw())
	{
		Draw->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (UBorder* Defeat = GetDefeat())
	{
		Defeat->SetVisibility(ESlateVisibility::Visible);
	}
	if (UTextBlock* Status = GetStatus())
	{
		Status->SetText(FText::FromString(TEXT("Otro jugador reventó primero — volviendo al hub...")));
	}
	BP_ShowDefeat();
}

void UBalloonBurstUI::ShowDraw()
{
	HideStartBanner();

	if (UBorder* Victory = GetVictory())
	{
		Victory->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (UBorder* Defeat = GetDefeat())
	{
		Defeat->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (UBorder* Draw = GetDraw())
	{
		Draw->SetVisibility(ESlateVisibility::Visible);
	}
	if (UTextBlock* Status = GetStatus())
	{
		Status->SetText(FText::FromString(TEXT("¡Empate! Varios globos reventaron a la vez — volviendo al hub...")));
	}
	BP_ShowDraw();
}

UTextBlock* UBalloonBurstUI::GetPlayers() const
{
	return PlayersText ? PlayersText.Get() : RuntimePlayersText.Get();
}

UTextBlock* UBalloonBurstUI::GetInstructions() const
{
	return InstructionsText ? InstructionsText.Get() : RuntimeInstructionsText.Get();
}

UTextBlock* UBalloonBurstUI::GetStatus() const
{
	return StatusText ? StatusText.Get() : RuntimeStatusText.Get();
}

UTextBlock* UBalloonBurstUI::GetCountdown() const
{
	return CountdownText ? CountdownText.Get() : RuntimeCountdownText.Get();
}

UTextBlock* UBalloonBurstUI::GetProgress() const
{
	return ProgressText ? ProgressText.Get() : RuntimeProgressText.Get();
}

UBorder* UBalloonBurstUI::GetVictory() const
{
	return VictoryBanner ? VictoryBanner.Get() : RuntimeVictoryBanner.Get();
}

UBorder* UBalloonBurstUI::GetDefeat() const
{
	return DefeatBanner ? DefeatBanner.Get() : RuntimeDefeatBanner.Get();
}

UBorder* UBalloonBurstUI::GetDraw() const
{
	return DrawBanner ? DrawBanner.Get() : RuntimeDrawBanner.Get();
}
