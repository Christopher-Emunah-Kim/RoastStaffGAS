// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "RS_DamageExecCalc.generated.h"

/**
 * URS_DamageExecCalc
 * GE_WeaponDamage / GE_EnemyDamage 공용 ExecCalc.
 * Source 팀 태그로 방향을 판별한 뒤 공식 분기.
 *
 *   플레이어→에너미: BaseDmg × (1 + ATK/100) × CritMult
 *   에너미→플레이어: max(1, EnemyDmg - DEF)
 *
 * SetByCaller 키:
 *   Data.WeaponBaseDamage  — GA_ProjectileAttack / BaseSummonObject 주입
 *   Data.EnemyAttackDamage — 에너미 공격 코드 주입
 */
UCLASS()
class ROASTSTAFFGAS_API URS_DamageExecCalc : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	URS_DamageExecCalc();

	virtual void Execute_Implementation(
		const FGameplayEffectCustomExecutionParameters& ExecutionParams,
		FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
