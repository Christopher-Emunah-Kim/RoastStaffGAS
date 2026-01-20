// Fill out your copyright notice in the Description page of Project Settings.


#include "Temp/Net_PickupItem.h"
#include "NavigationSystemTypes.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "System/K_LoggingSystem.h"
#include "Temp/Net_PlayerCharacter.h"

ANet_PickupItem::ANet_PickupItem()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	
	RootComponent = CreateDefaultSubobject<USceneComponent>("Root");
	
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
	CollisionComp->SetupAttachment(RootComponent);
	CollisionComp->SetSphereRadius(100.f);
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComp->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	
	StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	StaticMeshComp->SetupAttachment(CollisionComp);
	StaticMeshComp->SetCollisionProfileName(FName("NoCollision"));
}


void ANet_PickupItem::BeginPlay()
{
	Super::BeginPlay();
	
}

void ANet_PickupItem::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);
	
	//서버에서만 처리
	if (GetLocalRole() != ROLE_Authority)
	{
		return;
	}
	
	ANet_PlayerCharacter* player = Cast<ANet_PlayerCharacter>(OtherActor);
	if (!ensureMsgf(player, TEXT("Overlapped Actor is not player")))
	{
		return;
	}
	player->AddPickUp();
	
	//KHS_INFO(TEXT("Picked up by %s"), *player->GetName());
	
	//TODO : GC 재생
	
	Destroy();
	
}


