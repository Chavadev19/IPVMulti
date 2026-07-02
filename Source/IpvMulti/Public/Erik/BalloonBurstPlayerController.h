// Copyright © 2026 IpvMulti - Erik

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BalloonBurstPlayerController.generated.h"

class ABalloonBurstBalloon;

/**
 * Controlador minimo para Balloon Burst.
 * Bombea el globo con la barra espaciadora y opcionalmente usa una camara fija.
 */
UCLASS()
class IPVMULTI_API ABalloonBurstPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ABalloonBurstPlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	/** Globo que se inflara. Si esta vacio, se busca automaticamente en el nivel. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Balloon Burst")
	TObjectPtr<ABalloonBurstBalloon> TargetBalloon;

	/** Camara fija del nivel. Si se asigna, se usa como vista al iniciar. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Balloon Burst")
	TObjectPtr<AActor> FixedCameraActor;

private:
	void OnPumpPressed();
	void OnPumpReleased();
	void ResolveTargetBalloon();
};
