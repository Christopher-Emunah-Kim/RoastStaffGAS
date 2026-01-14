// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/K_BaseGameplayAbility.h"
#include "K_GA_Fireball.generated.h"

class AK_FireballProjectile;
/**
 * 
 */
UCLASS()
class ROASTSTAFFGAS_API UK_GA_Fireball : public UK_BaseGameplayAbility
{
	GENERATED_BODY()
public:
	UK_GA_Fireball();
	
protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
	UFUNCTION(BlueprintCallable, Category = "AM|Ability|Fireball")
	AK_FireballProjectile* SpawnFireBall(const FGameplayAbilityActorInfo& ActorInfo);
	
	
	UPROPERTY(EditDefaultsOnly, Category = "AM|Ability|Fireball")
	TSubclassOf<AK_FireballProjectile> ProjectileClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "AM|Ability|Fireball")
	float SpawnOffset = 100.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "AM|Ability|Fireball")
	float BaseFireballDamage = 10.f;
	
	
};
