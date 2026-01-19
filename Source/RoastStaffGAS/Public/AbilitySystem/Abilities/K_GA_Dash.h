// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/K_BaseGameplayAbility.h"
#include "K_GA_Dash.generated.h"

class UGameplayEffect;

/**
 * 
 */
UCLASS()
class ROASTSTAFFGAS_API UK_GA_Dash : public UK_BaseGameplayAbility
{
	GENERATED_BODY()
	
public:
	UK_GA_Dash();
	
protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	

	UPROPERTY(EditDefaultsOnly, Category="AM|Dash")
	float DashImpulse = 2500.f;
	
	UPROPERTY(EditDefaultsOnly, Category="AM|Dash|Effects")
	TSubclassOf<UGameplayEffect> DashEffect;
};
