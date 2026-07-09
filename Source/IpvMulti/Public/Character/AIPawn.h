// Copyright © 2026 IpvMulti

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AIPawn.generated.h"

class UPawnSensingComponent;

UCLASS()
class IPVMULTI_API AAIPawn : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AAIPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	virtual void PostInitializeComponents() override;
	
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UPawnSensingComponent> PawnSensingComp;

	UFUNCTION()
	void HandleOnSeenPawn(APawn* InPawn);
	
	UFUNCTION()
	void HandleOnHearNoise(APawn* InInstigator, const FVector& Location, float Volume);
	
	FRotator OriginalRotation;
	
	UFUNCTION()
	void ResetOrientation();
	
	FTimerHandle TimerHandle_ResetOrientation;
	
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
