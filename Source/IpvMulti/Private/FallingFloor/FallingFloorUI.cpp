// Copyright © 2026 IpvMulti

#include "FallingFloor/FallingFloorUI.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"

void UFallingFloorUI::NativeConstruct()
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
	if (UTextBlock* Status = GetStatus())
	{
		Status->SetText(FText::FromString(TEXT("Muévete — la partida empieza pronto")));
	}
	if (UTextBlock* Countdown = GetCountdown())
	{
		Countdown->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}

TSharedRef<SWidget> UFallingFloorUI::RebuildWidget()
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

	RuntimePlayersText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("PlayersRemainingText"));
	RuntimePlayersText->SetText(FText::FromString(TEXT("Jugadores: -")));
	FSlateFontInfo PlayersFont = RuntimePlayersText->GetFont();
	PlayersFont.Size = 36;
	RuntimePlayersText->SetFont(PlayersFont);
	RuntimePlayersText->SetColorAndOpacity(FLinearColor::White);
	if (UCanvasPanelSlot* PlayersSlot = Root->AddChildToCanvas(RuntimePlayersText))
	{
		PlayersSlot->SetAnchors(FAnchors(0.5f, 0.0f));
		PlayersSlot->SetAlignment(FVector2D(0.5f, 0.0f));
		PlayersSlot->SetPosition(FVector2D(0.0f, 40.0f));
		PlayersSlot->SetAutoSize(true);
	}

	RuntimeStatusText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StatusText"));
	RuntimeStatusText->SetText(FText::FromString(TEXT("Muévete — la partida empieza pronto")));
	FSlateFontInfo StatusFont = RuntimeStatusText->GetFont();
	StatusFont.Size = 22;
	RuntimeStatusText->SetFont(StatusFont);
	RuntimeStatusText->SetColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f));
	if (UCanvasPanelSlot* StatusSlot = Root->AddChildToCanvas(RuntimeStatusText))
	{
		StatusSlot->SetAnchors(FAnchors(0.5f, 0.0f));
		StatusSlot->SetAlignment(FVector2D(0.5f, 0.0f));
		StatusSlot->SetPosition(FVector2D(0.0f, 90.0f));
		StatusSlot->SetAutoSize(true);
	}

	RuntimeCountdownText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("CountdownText"));
	RuntimeCountdownText->SetText(FText::FromString(TEXT("5")));
	FSlateFontInfo CountdownFont = RuntimeCountdownText->GetFont();
	CountdownFont.Size = 96;
	RuntimeCountdownText->SetFont(CountdownFont);
	RuntimeCountdownText->SetJustification(ETextJustify::Center);
	RuntimeCountdownText->SetColorAndOpacity(FLinearColor(1.0f, 0.92f, 0.2f));
	if (UCanvasPanelSlot* CountdownSlot = Root->AddChildToCanvas(RuntimeCountdownText))
	{
		CountdownSlot->SetAnchors(FAnchors(0.5f, 0.35f));
		CountdownSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		CountdownSlot->SetPosition(FVector2D(0.0f, 0.0f));
		CountdownSlot->SetAutoSize(true);
	}

	RuntimeVictoryBanner = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("VictoryBanner"));
	RuntimeVictoryBanner->SetBrushColor(FLinearColor(0.05f, 0.45f, 0.15f, 0.85f));
	RuntimeVictoryBanner->SetVisibility(ESlateVisibility::Collapsed);
	UTextBlock* VictoryText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("VictoryText"));
	VictoryText->SetText(FText::FromString(TEXT("¡VICTORIA!")));
	FSlateFontInfo BannerFont = VictoryText->GetFont();
	BannerFont.Size = 64;
	VictoryText->SetFont(BannerFont);
	VictoryText->SetJustification(ETextJustify::Center);
	VictoryText->SetColorAndOpacity(FLinearColor::White);
	RuntimeVictoryBanner->SetContent(VictoryText);
	if (UCanvasPanelSlot* VictorySlot = Root->AddChildToCanvas(RuntimeVictoryBanner))
	{
		VictorySlot->SetAnchors(FAnchors(0.5f, 0.5f));
		VictorySlot->SetAlignment(FVector2D(0.5f, 0.5f));
		VictorySlot->SetPosition(FVector2D(0.0f, 0.0f));
		VictorySlot->SetSize(FVector2D(520.0f, 140.0f));
	}

	RuntimeDefeatBanner = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("DefeatBanner"));
	RuntimeDefeatBanner->SetBrushColor(FLinearColor(0.45f, 0.05f, 0.08f, 0.85f));
	RuntimeDefeatBanner->SetVisibility(ESlateVisibility::Collapsed);
	UTextBlock* DefeatText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("DefeatText"));
	DefeatText->SetText(FText::FromString(TEXT("DERROTA")));
	DefeatText->SetFont(BannerFont);
	DefeatText->SetJustification(ETextJustify::Center);
	DefeatText->SetColorAndOpacity(FLinearColor::White);
	RuntimeDefeatBanner->SetContent(DefeatText);
	if (UCanvasPanelSlot* DefeatSlot = Root->AddChildToCanvas(RuntimeDefeatBanner))
	{
		DefeatSlot->SetAnchors(FAnchors(0.5f, 0.5f));
		DefeatSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		DefeatSlot->SetPosition(FVector2D(0.0f, 0.0f));
		DefeatSlot->SetSize(FVector2D(520.0f, 140.0f));
	}

	return Super::RebuildWidget();
}

void UFallingFloorUI::UpdatePlayersRemaining(int32 PlayersRemaining)
{
	if (UTextBlock* Text = GetPlayersText())
	{
		Text->SetText(FText::FromString(FString::Printf(TEXT("Jugadores restantes: %d"), PlayersRemaining)));
	}
	BP_UpdatePlayersRemaining(PlayersRemaining);
}

void UFallingFloorUI::UpdateCountdown(int32 SecondsRemaining)
{
	if (UTextBlock* Countdown = GetCountdown())
	{
		if (SecondsRemaining > 0)
		{
			Countdown->SetVisibility(ESlateVisibility::HitTestInvisible);
			Countdown->SetText(FText::AsNumber(SecondsRemaining));
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
				FString::Printf(TEXT("Empieza en %d — ¡muévete!"), SecondsRemaining)));
		}
	}

	BP_UpdateCountdown(SecondsRemaining);
}

void UFallingFloorUI::NotifyMatchPhase(EFallingFloorMatchPhase NewPhase)
{
	if (UTextBlock* Status = GetStatus())
	{
		switch (NewPhase)
		{
		case EFallingFloorMatchPhase::WaitingToStart:
			Status->SetText(FText::FromString(TEXT("Muévete — la partida empieza pronto")));
			break;
		case EFallingFloorMatchPhase::InProgress:
			Status->SetText(FText::FromString(TEXT("¡Sobrevive!")));
			if (UTextBlock* Countdown = GetCountdown())
			{
				Countdown->SetVisibility(ESlateVisibility::Collapsed);
			}
			break;
		case EFallingFloorMatchPhase::Ended:
			Status->SetText(FText::FromString(TEXT("Volviendo al hub...")));
			break;
		}
	}
	BP_NotifyMatchPhase(NewPhase);
}

void UFallingFloorUI::OnMatchStarted(int32 PlayersAlive)
{
	UpdateCountdown(0);
	UpdatePlayersRemaining(PlayersAlive);
	NotifyMatchPhase(EFallingFloorMatchPhase::InProgress);
	BP_OnMatchStarted(PlayersAlive);
}

void UFallingFloorUI::ShowVictory()
{
	if (UBorder* Defeat = GetDefeat())
	{
		Defeat->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (UBorder* Victory = GetVictory())
	{
		Victory->SetVisibility(ESlateVisibility::Visible);
	}
	if (UTextBlock* Status = GetStatus())
	{
		Status->SetText(FText::FromString(TEXT("¡Eres el último en pie! Volviendo al hub...")));
	}
	BP_ShowVictory();
}

void UFallingFloorUI::ShowDefeat()
{
	if (UBorder* Victory = GetVictory())
	{
		Victory->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (UBorder* Defeat = GetDefeat())
	{
		Defeat->SetVisibility(ESlateVisibility::Visible);
	}
	if (UTextBlock* Status = GetStatus())
	{
		Status->SetText(FText::FromString(TEXT("Has caído — volviendo al hub...")));
	}
	BP_ShowDefeat();
}

UTextBlock* UFallingFloorUI::GetPlayersText() const
{
	return PlayersRemainingText ? PlayersRemainingText.Get() : RuntimePlayersText.Get();
}

UTextBlock* UFallingFloorUI::GetStatus() const
{
	return StatusText ? StatusText.Get() : RuntimeStatusText.Get();
}

UTextBlock* UFallingFloorUI::GetCountdown() const
{
	return CountdownText ? CountdownText.Get() : RuntimeCountdownText.Get();
}

UBorder* UFallingFloorUI::GetVictory() const
{
	return VictoryBanner ? VictoryBanner.Get() : RuntimeVictoryBanner.Get();
}

UBorder* UFallingFloorUI::GetDefeat() const
{
	return DefeatBanner ? DefeatBanner.Get() : RuntimeDefeatBanner.Get();
}
