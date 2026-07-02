// Copyright © 2026 IpvMulti - Erik

#include "Erik/BalloonBurstGameMode.h"

#include "Erik/BalloonBurstPlayerController.h"

ABalloonBurstGameMode::ABalloonBurstGameMode()
{
	PlayerControllerClass = ABalloonBurstPlayerController::StaticClass();
	DefaultPawnClass = nullptr;
}
