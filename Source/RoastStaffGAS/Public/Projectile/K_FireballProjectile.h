// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "K_FireballProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UAbilitySystemComponent;

UCLASS()
class ROASTSTAFFGAS_API AK_FireballProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AK_FireballProjectile();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
public:
	UFUNCTION(BlueprintCallable, Category = "AM|Damage")
	void SetDamageInfo(float InBaseDamage, UAbilitySystemComponent* InOwnerASC);
	

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AM|Comp")
	TObjectPtr<USphereComponent> SphereComp;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AM|Comp")
	TObjectPtr<UProjectileMovementComponent> ProjectileComp;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AM|Damage")
	float BaseDamage;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AM|Damage")
	TObjectPtr<UAbilitySystemComponent> OwnerASC;
	
};
