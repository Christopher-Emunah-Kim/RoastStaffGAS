// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/K_BaseGameplayAbility.h"
#include "K_GA_BasicShoot.generated.h"

class AK_BasicShootProjectile;
/**
 * 기본 사격 어빌리티
 */
UCLASS()
class ROASTSTAFFGAS_API UK_GA_BasicShoot : public UK_BaseGameplayAbility
{
	GENERATED_BODY()

public:
	UK_GA_BasicShoot();
	
protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
	UFUNCTION(BlueprintCallable, Category = "AM|GAS")
	AK_BasicShootProjectile* SpawnBasicProjectile(const FGameplayAbilityActorInfo& ActorInfo);
	
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "AM|GAS")
	TSubclassOf<AK_BasicShootProjectile> ProjectileClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "AM|GAS")
	float SpawnOffset = 100.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "AM|GAS")
	float BaseDamage = 5.f;
	
	
};
