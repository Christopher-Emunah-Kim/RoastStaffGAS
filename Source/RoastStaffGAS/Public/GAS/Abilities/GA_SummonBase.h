// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/DataTableStructs.h"
#include "Data/RuntimeDataStructs.h"
#include "GAS/Abilities/GA_Base.h"
#include "GA_SummonBase.generated.h"

/**
  * UGA_SummonBase
  * 소환형 스킬 베이스 GA.
  * DetermineSummonLocation
  * EndAbility 오버라이드로 쿨타임 타이머 재시작 보장.
  */

class ASummonPreviewObject;

UCLASS()
class ROASTSTAFFGAS_API UGA_SummonBase : public UGA_Base
{
	GENERATED_BODY()
	
public:
	UGA_SummonBase();

protected:
	virtual void OnAbilityActivated(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	//소환 위치 결정
	virtual FVector DetermineSummonLocation(); 
	//액티브 모드 처리
	virtual void HandleActiveMode();                                                                           
	
private:
	//위치에 오브젝트 소환 헬퍼
	void SpawnSummonObject(const FVector& Location);
	//데이터 조회/세팅 헬퍼
	bool LoadSkillData();
	bool LoadSummonData(FSkillExecutionData& OutExecData, FSkillAttackMoveTypeParamsSummon& OutSummonParam) const;
	bool SetSummonData(FSummonObjectInitData& InitData, TSubclassOf<AActor>& SummonClass);
	//프리뷰 오브젝트 소환
	void SpawnPreviewObject();
	//자동 모드: SearchRange 내 최근접 적 탐색
	void FindNearestEnemy(AActor*& OutEnemy) const;
	//액티브 모드 체크 헬퍼
	bool CheckIsActiveSlot() const;
	//액티브 모드 입력처리 헬퍼
	//(UAbilityTask_WaitConfirmCancel)
	UFUNCTION()
	void OnConfirm();
	UFUNCTION()
	void OnCancel();
	
protected:
	TWeakObjectPtr<ASummonPreviewObject> CachedPreviewObject;
	
	FName CachedSkillID;
	FSkillExecutionData CachedExecData;
	FSkillAttackMoveTypeParamsSummon CachedSummonParam;
	
};
