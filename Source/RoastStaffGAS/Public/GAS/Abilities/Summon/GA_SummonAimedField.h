// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/GA_SummonBase.h"
#include "GA_SummonAimedField.generated.h"

/**
  * UGA_SummonAimedField
  * 에임 장판형.
  * - Active(수동) 모드: SummonPreviewObject 스폰 → WaitConfirmCancel → 좌클릭 시 소환
  * - 자동 모드: CachedAimLocation(커서 위치)에 즉시 소환
  */

class ASummonPreviewObject;

UCLASS()
class ROASTSTAFFGAS_API UGA_SummonAimedField : public UGA_SummonBase
{
	GENERATED_BODY()
	
protected:
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
	virtual FVector DetermineSummonLocation() override;
	virtual void HandleActiveMode() override;
	
private:
	void SpawnPreviewObject();

protected:
	UPROPERTY()
	TObjectPtr<ASummonPreviewObject> CachedPreviewObject;
	
};
