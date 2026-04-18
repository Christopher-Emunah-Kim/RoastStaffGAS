// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/GA_Base.h"
#include "Data/RuntimeDataStructs.h"
#include "GA_CharacterSkill.generated.h"

class UNiagaraSystem;
class ABaseProjectile;
class AGroundEffectActor;

/**
 * UGA_CharacterSkill
 * 캐릭터 고유 스킬 GA.
 * InstantAoE / SelfBuff / SpawnPreview / ProjectileSpawn 처리.
 */
UCLASS()
class ROASTSTAFFGAS_API UGA_CharacterSkill : public UGA_Base
{
	GENERATED_BODY()

protected:
	virtual void OnAbilityActivated(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) override;

private:
	/** InstantAoE: 시전자 위치 기준 구형 범위 내 적에게 GE 적용 */
	void ExecuteInstantAoE(
		const FCharacterSkillExecData& ExecData,
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo);

	/** SelfBuff: 시전자 자신에게 지속시간 GE 적용 */
	void ExecuteSelfBuff(
		const FCharacterSkillExecData& ExecData,
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo);

	/** SpawnPreview: SkillManagerSubsystem이 저장한 PendingTargetLocation 기준 AoE 적용 */
	void ExecuteSpawnPreview(
		const FCharacterSkillExecData& ExecData,
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo);

	/**
	 * ProjectileSpawn: 투사체 직접 발사 (단발 or 연속)
	 * ProjectileCount > 1인 경우 FireInterval 타이머로 연속 발사 후 EndAbility
	 */
	void ExecuteProjectileSpawn(
		const FCharacterSkillExecData& ExecData,
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo);

	/**
	 * GroundEffect: PoolingSubsystem에서 AARS_GroundEffectActor를 꺼내
	 * PendingTargetLocation에 배치 후 즉시 EndAbility.
	 * 장판 Actor가 독립 수명 타이머로 Duration 후 ReturnToPool.
	 */
	void ExecuteGroundEffect(
		const FCharacterSkillExecData& ExecData,
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo);

	/** 단발 투사체 조립 + SpawnProjectiles 호출 헬퍼 */
	void FireOneProjectile(TSubclassOf<ABaseProjectile> ProjClass, const FCharacterSkillExecData& ExecData);

	/**
	 * FXClass를 Location에 스폰, Radius + ElementColor(ElementTag 기반) 파라미터 주입.
	 * ElementTag 없으면 White. Niagara FX에 "Radius"(float), "ElementColor"(LinearColor) 파라미터 필수.
	 */
	void SpawnSkillFX(TSoftObjectPtr<UNiagaraSystem> FXClass, FVector Location, float Radius, FGameplayTag ElementTag = FGameplayTag());

protected:
	/**
	 * 스킬 효과 GE 클래스 — BP에서 슬롯에 맞게 설정 필수.
	 *   InstantAoE / SpawnPreview: Data.WeaponBaseDamage SetByCaller → GE_Damage(ExecCalc) 권장
	 *   SelfBuff: 버프 속성 GE (Duration = Instant 이외)
	 *   ProjectileSpawn: 투사체 충돌 시 적용할 GE (단일 데미지 GE)
	 */
	UPROPERTY(EditDefaultsOnly, Category = "MY|CharacterSkill")
	TSubclassOf<UGameplayEffect> SkillGEClass;

	static constexpr float DESTROY_FX_DELAY = 2.0f;

private:
	// ── ProjectileSpawn 연속 발사 상태 ──────────────────────────────────────

	/** 연속 발사 시 캐시된 투사체 클래스 (로드 완료) */
	UPROPERTY()
	TSubclassOf<ABaseProjectile> ActiveProjClass;

	/** 연속 발사 시 캐시된 ExecData */
	UPROPERTY()
	FCharacterSkillExecData CachedProjExecData;

	/** 연속 발사 남은 횟수 */
	int32 RemainingFireCount = 0;

	/** 연속 발사 간격 타이머 */
	FTimerHandle MultiFireTimerHandle;
};
