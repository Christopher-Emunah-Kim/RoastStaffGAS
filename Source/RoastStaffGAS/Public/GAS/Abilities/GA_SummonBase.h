// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/DataTableStructs.h"
#include "Data/RuntimeDataStructs.h"
#include "GAS/Abilities/GA_Base.h"
#include "GA_SummonBase.generated.h"

/**
  * UGA_SummonBase
  * 소환형 스킬 추상 베이스 GA.
  * DetermineSummonLocation: 자식 구현 (PURE_VIRTUAL)
  * EndAbility 오버라이드로 쿨타임 타이머 재시작 보장.
  */

UCLASS(Abstract)
class ROASTSTAFFGAS_API UGA_SummonBase : public UGA_Base
{
	GENERATED_BODY()
	
public:
	UGA_SummonBase();
	
protected:
	virtual void OnAbilityActivated(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	//소환 위치 결정(순수가상함수)
	virtual FVector DetermineSummonLocation() PURE_VIRTUAL(UGA_SummonBase::DetermineSummonLocation, return FVector::ZeroVector;) 
	
	//위치에 오브젝트 소환
	void SpawnSummonObject(const FVector& Location);
	//필요 데이터 조회
	bool LoadSummonData(FSkillExecutionData& OutExecData, FSkillAttackMoveTypeParamsSummon& OutSummonParam) const;
	//소환 데이터 세팅
	bool SetSummonData(FSummonObjectInitData& InitData, FActorSpawnParameters& SpawnParams, TSubclassOf<AActor>& SummonClass);
	
	
protected:
	FName CachedSkillID;
	FSkillExecutionData CachedExecData;
	FSkillAttackMoveTypeParamsSummon CachedSummonParam;
	
};
