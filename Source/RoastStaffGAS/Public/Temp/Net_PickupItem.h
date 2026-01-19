// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Net_PickupItem.generated.h"

class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class ROASTSTAFFGAS_API ANet_PickupItem : public AActor
{
	GENERATED_BODY()
	
public:	
	ANet_PickupItem();

protected:
	virtual void BeginPlay() override;

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Comp")
	TObjectPtr<USphereComponent> CollisionComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Comp")
	TObjectPtr<UStaticMeshComponent> StaticMeshComp;
	
};
