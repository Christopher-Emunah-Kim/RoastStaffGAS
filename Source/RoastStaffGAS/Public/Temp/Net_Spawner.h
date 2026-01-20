// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Net_Spawner.generated.h"

class ANet_PickupItem;

UCLASS()
class ROASTSTAFFGAS_API ANet_Spawner : public AActor
{
	GENERATED_BODY()
	
public:	
	ANet_Spawner();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void SpawnPickup();

protected:
	UPROPERTY(EditAnywhere, Category = "AM|Spawner")
	TSubclassOf<ANet_PickupItem> PickUpClass;
	
	UPROPERTY(EditAnywhere, Category = "AM|Spawner")
	float SpawnInterval;
	
	UPROPERTY(EditAnywhere, Category = "AM|Spawner")
	float SpawnRadius;

	FTimerHandle SpawnTimer;
};
