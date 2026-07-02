// Copyright © 2026 IpvMulti - Erik

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BalloonBurstBalloon.generated.h"

class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBalloonBurst);

/**
 * Esfera inflable para el minijuego Balloon Burst.
 * Crece mientras se bombea y explota al alcanzar el tamano maximo.
 */
UCLASS()
class IPVMULTI_API ABalloonBurstBalloon : public AActor
{
	GENERATED_BODY()

public:
	ABalloonBurstBalloon();

	/** Empieza a inflar el globo (mantener barra espaciadora). */
	UFUNCTION(BlueprintCallable, Category = "Balloon Burst")
	void StartInflating();

	/** Deja de inflar el globo. */
	UFUNCTION(BlueprintCallable, Category = "Balloon Burst")
	void StopInflating();

	/** Reinicia el globo a su tamano inicial. */
	UFUNCTION(BlueprintCallable, Category = "Balloon Burst")
	void ResetBalloon();

	UFUNCTION(BlueprintPure, Category = "Balloon Burst")
	bool HasBurst() const { return bHasBurst; }

	UFUNCTION(BlueprintPure, Category = "Balloon Burst")
	float GetCurrentScale() const { return CurrentScale; }

	UPROPERTY(BlueprintAssignable, Category = "Balloon Burst")
	FOnBalloonBurst OnBalloonBurst;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> BalloonMesh;

	/** Escala minima del globo (tamaño en reposo). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Balloon Burst", meta = (ClampMin = "0.01"))
	float MinScale = 1.0f;

	/** Escala maxima antes de explotar. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Balloon Burst", meta = (ClampMin = "0.01"))
	float MaxScale = 3.0f;

	/** Unidades de escala que gana por segundo mientras se bombea. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Balloon Burst", meta = (ClampMin = "0.01"))
	float InflationRate = 0.75f;

	/** Si es true, el globo se destruye al explotar. Si es false, solo se oculta. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Balloon Burst")
	bool bDestroyOnBurst = false;

	UFUNCTION(BlueprintImplementableEvent, Category = "Balloon Burst")
	void OnBurstEffects();

private:
	void ApplyScale(float NewScale);
	void Burst();

	bool bIsInflating = false;
	bool bHasBurst = false;
	float CurrentScale = 1.0f;
};
