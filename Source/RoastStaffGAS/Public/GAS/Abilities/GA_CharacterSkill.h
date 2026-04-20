// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/GA_Base.h"
#include "Data/RuntimeDataStructs.h"
#include "GA_CharacterSkill.generated.h"

class UNiagaraSystem;
class ABaseProjectile;
class AGroundEffectActor;
class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitGameplayEvent;

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

	/**
	 * 몽타주가 할당된 경우: 몽타주 재생 + 이동 잠금 → HitCheck 노티파이 대기 → Execute
	 * 몽타주 없는 경우: 즉시 Execute
	 */
	void StartSkillWithMontage(
		const FCharacterSkillExecData& ExecData,
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		TFunction<void()> ExecuteFunc);

	/** AbilityTask_PlayMontageAndWait 완료/취소/중단 콜백 — 이동 복원 + EndAbility */
	UFUNCTION()
	void OnCastingMontageEnded();

	/** AbilityTask_WaitGameplayEvent — HitCheck 노티파이 수신 콜백 */
	UFUNCTION()
	void OnHitCheckReceived(FGameplayEventData Payload);

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
	 * FXLifetime: 0 이하면 DESTROY_FX_DELAY 사용 (기본값).
	 * Rotation: FX 스폰 방향 (기본값 ZeroRotator).
	 */
	void SpawnSkillFX(TSoftObjectPtr<UNiagaraSystem> FXClass, FVector Location, float Radius,
		FGameplayTag ElementTag = FGameplayTag(), float FXLifetime = 0.f, FRotator Rotation = FRotator::ZeroRotator);

protected:
	/**
	 * 스킬 효과 GE 클래스 — BP에서 슬롯에 맞게 설정 필수.
	 *   InstantAoE / SpawnPreview: Data.WeaponBaseDamage SetByCaller → GE_Damage(ExecCalc) 권장
	 *   SelfBuff: 버프 속성 GE (Duration = Instant 이외)
	 *   ProjectileSpawn: 투사체 충돌 시 적용할 GE (단일 데미지 GE)
	 */
	UPROPERTY(EditDefaultsOnly, Category = "MY|CharacterSkill")
	TSubclassOf<UGameplayEffect> SkillGEClass;
	/**
	 * 스킬 시전 몽타주 — 할당 시 몽타주 재생 후 HitCheck 노티파이에서 효과 발동.
	 * 미할당 시 즉시 발동 (기존 동작 유지).
	 * BP에서 스킬별로 할당.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "MY|CharacterSkill")
	TObjectPtr<UAnimMontage> CastingMontage;
	/**
	 * FX Actor 클래스 — 할당 시 SpawnSkillFX 대신 BP 액터를 스폰.
	 * BP 내부에서 NiagaraComponent 로컬 회전을 자유롭게 설정 가능.
	 * 미할당 시 기존 SpawnSkillFX(NiagaraSystem 직접 스폰) 동작 유지.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "MY|CharacterSkill")
	TSubclassOf<AActor> FXActorClass;

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

	// ── 몽타주 캐스팅 상태 ────────────────────────────────────────────────────
	/** HitCheck 노티파이 수신 시 호출할 Execute 함수 (몽타주 있을 때만 사용) */
	TFunction<void()> PendingExecuteFunc;
	/** HitCheck 중복 호출 방지 플래그 */
	bool bExecuteFuncCalled = false;
};
