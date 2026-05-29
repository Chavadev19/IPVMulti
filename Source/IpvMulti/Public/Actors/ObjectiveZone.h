// Copyright © 2026 IpvMulti

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ObjectiveZone.generated.h"

class UBoxComponent;

UCLASS()
class IPVMULTI_API AObjectiveZone : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AObjectiveZone();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	UPROPERTY(VisibleAnywhere, Category = "Component")
	TObjectPtr<UBoxComponent> OverlapComp;
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UDecalComponent> DecalComp;
	UFUNCTION()
	void HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

public:

};
