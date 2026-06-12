// Copyright Epic Games, Inc. All Rights Reserved.

#include "IpvMultiHealthWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/ProgressBar.h"

void UIpvMultiHealthWidget::NativeConstruct()
{
	Super::NativeConstruct();
	EnsureHealthProgressBar();
}

void UIpvMultiHealthWidget::EnsureHealthProgressBar()
{
	if (HealthProgressBar)
	{
		return;
	}

	if (!WidgetTree)
	{
		return;
	}

	UCanvasPanel* RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
	WidgetTree->RootWidget = RootCanvas;

	UBorder* HealthBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("HealthBorder"));
	HealthBorder->SetBrushColor(FLinearColor(0.05f, 0.05f, 0.05f, 0.85f));
	HealthBorder->SetPadding(FMargin(4.f));

	HealthProgressBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("HealthProgressBar"));
	HealthProgressBar->SetFillColorAndOpacity(FLinearColor(0.f, 0.85f, 0.1f, 1.f));
	HealthProgressBar->SetPercent(1.f);
	HealthBorder->AddChild(HealthProgressBar);

	if (UCanvasPanelSlot* BarSlot = RootCanvas->AddChildToCanvas(HealthBorder))
	{
		BarSlot->SetAnchors(FAnchors(0.f, 0.f));
		BarSlot->SetAlignment(FVector2D(0.f, 0.f));
		BarSlot->SetAutoSize(true);
		BarSlot->SetPosition(FVector2D(24.f, 24.f));
		BarSlot->SetSize(FVector2D(320.f, 32.f));
	}

	SetDesiredSizeInViewport(FVector2D(400.f, 80.f));
}

void UIpvMultiHealthWidget::UpdateHealth(float Current, float Max)
{
	EnsureHealthProgressBar();

	if (HealthProgressBar)
	{
		const float Percent = Max > 0.f ? FMath::Clamp(Current / Max, 0.f, 1.f) : 0.f;
		HealthProgressBar->SetPercent(Percent);
	}
}
