// Copyright Epic Games, Inc. All Rights Reserved.


#include "IpvMultiPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "IpvMulti.h"
#include "IpvMultiCharacter.h"
#include "IpvMultiHealthWidget.h"
#include "Widgets/Input/SVirtualJoystick.h"

AIpvMultiPlayerController::AIpvMultiPlayerController()
{
	HealthWidgetClass = UIpvMultiHealthWidget::StaticClass();
}

void AIpvMultiPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// only spawn touch controls on local player controllers
	if (ShouldUseTouchControls() && IsLocalPlayerController())
	{
		// spawn the mobile controls widget
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			// add the controls to the player screen
			MobileControlsWidget->AddToPlayerScreen(0);

		} else {

			UE_LOG(LogIpvMulti, Error, TEXT("Could not spawn mobile controls widget."));

		}

	}
}

void AIpvMultiPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

}

void AIpvMultiPlayerController::EnsureHealthWidget()
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	if (!HealthWidgetClass)
	{
		HealthWidgetClass = UIpvMultiHealthWidget::StaticClass();
	}

	if (HealthWidget)
	{
		return;
	}

	HealthWidget = CreateWidget<UIpvMultiHealthWidget>(this, HealthWidgetClass);
	if (HealthWidget)
	{
		HealthWidget->AddToViewport(10);
	}
	else
	{
		UE_LOG(LogIpvMulti, Error, TEXT("Could not spawn health HUD widget."));
	}
}

void AIpvMultiPlayerController::UpdateHealthDisplay(float Current, float Max)
{
	EnsureHealthWidget();

	if (HealthWidget)
	{
		HealthWidget->UpdateHealth(Current, Max);
	}
}

void AIpvMultiPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!ShouldUseTouchControls())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}
}

bool AIpvMultiPlayerController::ShouldUseTouchControls() const
{
	// are we on a mobile platform? Should we force touch?
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}
