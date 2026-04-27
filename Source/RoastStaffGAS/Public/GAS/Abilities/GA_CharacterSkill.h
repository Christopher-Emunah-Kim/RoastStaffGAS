// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/GA_Base.h"
#include "Data/RuntimeDataStructs.h"
#include "GA_CharacterSkill.generated.h"

class UNiagaraSystem;
class ABaseProjectile;
class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitGameplayEvent;
struct FProjectileInitData;

/**
 * UGA_CharacterSkill
 * 캐릭터 고유 스킬 GA.
 * 3축(TargetingType × EffectType × ProjectileMoveType) 분기 — ResolveTargeting / ResolveEffect 2단계 처리.
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

	/** Lerp 진행 중 외부 취소(피격 CC 등) 시 타이머 정리 + 이동 복원 보장 */
	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;
	

private:
	// ── Targeting 진입점 ──────────────────────────────────────────────────────
	/** AimPreview: 프리뷰 위치를 SkillManagerSubsystem에서 읽어 ResolveEffect 호출 */
	void ResolveTargeting_AimPreview(
		const FCharacterSkillExecData& ExecData,
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo);

	/** LaunchProjectile: SpawnPattern/SpawnCount 기반 투사체 발사 */
	void ResolveTargeting_LaunchProjectile(
		const FCharacterSkillExecData& ExecData,
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo);

	// ── Effect 실행 ───────────────────────────────────────────────────────────
	/** EffectType 기반 효과 실행 통합 진입점 */
	void ResolveEffect(
		const FCharacterSkillExecData& ExecData,
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		FVector TargetLocation);

	/** RadialAoE: 중심 위치 기준 구형 범위 내 적에게 GE 적용 */
	void ExecuteEffect_RadialAoE(
		const FCharacterSkillExecData& ExecData,
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		FVector TargetLocation);

	/** SelfBuff: 시전자 자신에게 지속시간 GE 적용 */
	void ExecuteEffect_SelfBuff(
		const FCharacterSkillExecData& ExecData,
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo);

	/** Teleport: TargetLocation으로 이동 + 출발/도착 FX */
	void ExecuteEffect_Teleport(
		const FCharacterSkillExecData& ExecData,
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		FVector TargetLocation);

	/** SpawnActor: PoolingSubsystem에서 EffectActorClass 꺼내 InitEffect 호출 */
	void ExecuteEffect_SpawnActor(
		const FCharacterSkillExecData& ExecData,
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		FVector TargetLocation);

	/** Projectile: 투사체 1발 조립 + SpawnProjectiles 호출 */
	void ExecuteEffect_Projectile(
		const FCharacterSkillExecData& ExecData,
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo);

	/** 백스텝샷: 가장 가까운 적 방향 폭발 투사체 발사 + 후방 Lerp 이동 + SelfBuff → EndAbility */
	void ExecuteEffect_BackstepShot(
		const FCharacterSkillExecData& ExecData,
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo);

	/** PlayerLoc 기준 SearchRadius 내 가장 가까운 적 반환. 없으면 nullptr. */
	AActor* FindNearestEnemy(FVector PlayerLoc, float SearchRadius) const;

protected:
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
	
	// ── 서브클래스 공유 헬퍼 ─────────────────────────────────────────────────
	/** ExecData → FProjectileInitData 조립 (ProjectileMoveType → EMoveType/EHitType 매핑 포함) */
	FProjectileInitData BuildProjectileInitData(const FCharacterSkillExecData& ExecData, TSubclassOf<ABaseProjectile> ProjClass) const;

	/** ATK × DamageMultiplier로 최종 데미지 산출 (캐릭터 스킬 경로) */
	float GetSkillDamageAmount(const FCharacterSkillExecData& ExecData) const;

	/**
	 * FXClass를 Location에 스폰, Radius + ElementColor(ElementTag 기반) 파라미터 주입.
	 * FXLifetime: 0 이하면 DESTROY_FX_DELAY 사용 (기본값).
	 */
	void SpawnSkillFX(TSoftObjectPtr<UNiagaraSystem> FXClass, FVector Location, float Radius,
		FGameplayTag ElementTag = FGameplayTag(), float FXLifetime = 0.f, FRotator Rotation = FRotator::ZeroRotator);


private:
	/** ElementTag → FLinearColor 변환. SpawnSkillFX + InitData 조립에서 공통 사용. */
	static FLinearColor ResolveElementColor(FGameplayTag ElementTag);

	/** 시전자를 TargetLocation까지 LerpDuration 동안 Lerp 이동. */
	void StartLerpMove(FVector TargetLocation, TFunction<void()> OnComplete);

protected:
	static constexpr float DESTROY_FX_DELAY = 2.0f;
	/** 백스텝샷 Lerp 이동 소요 시간 (초). DT 불필요 — 연출 고정값. */
	static constexpr float LerpDuration = 1.0f;

	/** 스킬 시전 몽타주 — 미할당 시 즉시 발동 */
	UPROPERTY(EditDefaultsOnly, Category = "MY|CharacterSkill")
	TObjectPtr<UAnimMontage> CastingMontage;
	/** FX Actor 클래스 — 할당 시 SpawnSkillFX 대신 BP 액터를 스폰 */
	UPROPERTY(EditDefaultsOnly, Category = "MY|CharacterSkill")
	TSubclassOf<AActor> FXActorClass;
	
private:
	// ── 백스텝샷 Lerp 이동 상태 ──────────────────────────────────────────────
	FTimerHandle LerpTimerHandle;
	FVector LerpStartLocation;
	FVector LerpTargetLocation;
	float LerpElapsed = 0.f;
	TFunction<void()> LerpOnComplete;
	/** true 동안 OnCastingMontageEnded가 EndAbility를 건너뜀 — lerp 콜백이 종료 처리. */
	bool bLerpInProgress = false;

	// ── LaunchProjectile 연속 발사 상태 ─────────────────────────────────────
	UPROPERTY()
	TSubclassOf<ABaseProjectile> ActiveProjClass;
	UPROPERTY()
	FCharacterSkillExecData CachedProjExecData;
	int32 RemainingFireCount = 0;
	FTimerHandle MultiFireTimerHandle;

	// ── 몽타주 캐스팅 상태 ────────────────────────────────────────────────────
	TFunction<void()> PendingExecuteFunc;
	bool bExecuteFuncCalled = false;
};
