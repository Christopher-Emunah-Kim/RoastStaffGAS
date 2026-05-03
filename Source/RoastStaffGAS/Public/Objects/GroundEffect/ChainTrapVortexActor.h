// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/PoolableInterface.h"
#include "Interface/SkillEffectInterface.h"
#include "ChainTrapVortexActor.generated.h"

class UAbilitySystemComponent;
class UGameplayEffect;
class UNiagaraSystem;
class UNiagaraComponent;

/**
 * AChainTrapVortexActor
 * 수렴 끌어당김 + 기절 설치 Actor.
 *
 * InitEffect() 호출 시:
 *   - SpawnFX (BP 설정): 스폰 즉시 재생
 *   - PullTick (0.1s looping): EffectRadius 내 적 XY 수렴 이동 (Z=0으로 바닥 뚫림 방지)
 *   - DurationTimer (1.0s): BurstFX 재생 → Instant 데미지 GE(SkillGEClass) + Stun GE(StatusGEClass) Apply → ReturnToPool
 *
 * EditDefaultsOnly 파라미터 (FX 포함)는 BP에서 스킬별로 설정.
 */
UCLASS()
class ROASTSTAFFGAS_API AChainTrapVortexActor : public AActor, public IPoolableInterface, public ISkillEffectInterface
{
	GENERATED_BODY()

public:
	AChainTrapVortexActor();
	virtual void BeginPlay() override;

	// ── IPoolableInterface ──────────────────────────────────────────────────
	virtual void OnPoolActivate() override;
	virtual void OnPoolDeactivate() override;

	// ── ISkillEffectInterface ───────────────────────────────────────────────
	virtual void InitEffect(const FSkillEffectInitData& InitData) override;

private:
	/** 매 PullTickRate마다 EffectRadius 내 적을 중심으로 수렴 이동 */
	void PullTick();

	/** DurationTimer 만료: Collision 복원 → BurstFX → GE Apply → ReturnToPool */
	void OnDurationExpired();

	void ApplyEffectsToTargets();
	void ApplyDamageGEToTarget(AActor* TargetActor);
	void ApplyGEToTarget(AActor* TargetActor, TSubclassOf<UGameplayEffect> GEClass);
	void ReturnToPool();

protected:
	/** 수렴 틱 간격 (초) */
	UPROPERTY(EditDefaultsOnly, Category = "ChainTrap")
	float PullTickRate = 0.1f;

	/** 수렴 속도 배율. 거리 * PullStrength = 매 틱 이동량 */
	UPROPERTY(EditDefaultsOnly, Category = "ChainTrap")
	float PullStrength = 2.5f;

	/** SpawnFX 재생 시간 (초) — 이 딜레이 후 수렴 시작 */
	UPROPERTY(EditDefaultsOnly, Category = "ChainTrap")
	float SpawnFXDuration = 2.0f;

	/** 스폰 즉시 재생 FX (루핑 Niagara 권장) */
	UPROPERTY(EditDefaultsOnly, Category = "ChainTrap|FX")
	TObjectPtr<UNiagaraSystem> SpawnFX;

	/** 수렴 완료 + 기절 시점 재생 FX (일회성 Niagara) */
	UPROPERTY(EditDefaultsOnly, Category = "ChainTrap|FX")
	TObjectPtr<UNiagaraSystem> BurstFX;

private:
	// ── 런타임 캐시 ─────────────────────────────────────────────────────────
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> CachedInstigatorASC;

	UPROPERTY()
	TSubclassOf<UGameplayEffect> CachedDamageGEClass;

	UPROPERTY()
	TSubclassOf<UGameplayEffect> CachedStunGEClass;

	float CachedEffectRadius = 0.f;
	float CachedAmount       = 0.f;

	/** PullTick에서 수집된 적 목록 — OnDurationExpired에서 GE Apply 대상 */
	UPROPERTY()
	TArray<TObjectPtr<AActor>> PulledEnemies;

	UPROPERTY()
	TObjectPtr<UNiagaraComponent> SpawnedFXComp;

	FTimerHandle SpawnFXDelayHandle;
	FTimerHandle PullTimerHandle;
	FTimerHandle DurationTimerHandle;
};
