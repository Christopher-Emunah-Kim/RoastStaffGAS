// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/GA_SummonBase.h"
#include "GA_SummonAutoTarget.generated.h"

/**
   * UGA_SummonAutoTarget
   * 즉발 소환형 — 자동/수동 모두 SearchRange 내 가장 가까운 적 위치에 즉시 소환.
   */

UCLASS()
class ROASTSTAFFGAS_API UGA_SummonAutoTarget : public UGA_SummonBase
{
	GENERATED_BODY()

protected:
	virtual void OnAbilityActivated(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	virtual FVector DetermineSummonLocation() override;

	const float FindNearestEnemy(AActor*& NearestEnemy, float& NearestDistSq);
	
private:                                                                                                         
	UFUNCTION()                                                                                                
	void OnConfirm();                                                                                            
	UFUNCTION()                                                                                                  
	void OnCancel();
                                                                                                                   
	bool CheckIsActiveSlot() const;  
};
