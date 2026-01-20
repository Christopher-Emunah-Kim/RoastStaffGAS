// Fill out your copyright notice in the Description page of Project Settings.


#include "Temp/Net_Spawner.h"

#include "NavigationSystem.h"
#include "RoastStaffGAS.h"
#include "Temp/Net_PickupItem.h"

// Sets default values
ANet_Spawner::ANet_Spawner()
	:SpawnInterval(3.f), SpawnRadius(1000.f)
{
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void ANet_Spawner::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		GetWorld()->GetTimerManager().SetTimer(
			SpawnTimer, this, &ANet_Spawner::SpawnPickup, SpawnInterval, true);
	}
	
	KHS_INFO(TEXT("[NetSpawner] Started spawning every %.1f seconds"), SpawnInterval);
}

void ANet_Spawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorld()->GetTimerManager().ClearTimer(SpawnTimer);
	Super::EndPlay(EndPlayReason);
}

void ANet_Spawner::SpawnPickup()
{
	if (!ensureMsgf(PickUpClass, TEXT("Pickup Class is not set on BP")))
	{
		return;
	}
	
	FVector randomLocation;
	UNavigationSystemV1* navSys = UNavigationSystemV1::GetCurrent(GetWorld());
	
	if (!ensureMsgf(navSys, TEXT("Failed to get current navigation system")))
	{
		return;
	}
	
	bool bSuccess = navSys->K2_GetRandomReachablePointInRadius(GetWorld(), GetActorLocation(), randomLocation, SpawnRadius);
	
	if (!bSuccess)
	{
		KHS_WARN(TEXT("Failed to get random spawn location"));
		return;
	}
	
	FActorSpawnParameters spawnParams;
	spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	
	ANet_PickupItem* pickUp = GetWorld()->SpawnActor<ANet_PickupItem>(
		PickUpClass, randomLocation, FRotator::ZeroRotator, spawnParams);
	
	if (!ensureMsgf(pickUp, TEXT("failed to spawn pickup item")))
	{
		return;
	}
	
	KHS_INFO(TEXT("Spawned pickup item"));
}


