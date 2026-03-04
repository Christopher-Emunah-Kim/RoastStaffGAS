// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "K_BasicShootProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UAbilitySystemComponent;
class UGameplayEffect;

UCLASS()
class ROASTSTAFFGAS_API AK_BasicShootProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AK_BasicShootProjectile();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	bool ApplyDamageToTarget(UAbilitySystemComponent* TargetASC);
	
public:	
	UFUNCTION(BlueprintCallable, Category = "AM|Damage")
	void SetDamageInfo(float InBaseDamage, UAbilitySystemComponent* InOwnerASC);

protected:
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> OwnerASC;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AM|Comp")
	TObjectPtr<USphereComponent> SphereComp;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AM|Comp")
	TObjectPtr<UProjectileMovementComponent> ProjectileComp;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AM|GAS|Effect")
	TSubclassOf<UGameplayEffect> DamageEffect;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AM|Damage")
	float BaseDamage;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AM|Projectile")
	float LifeSpan;
	
};
