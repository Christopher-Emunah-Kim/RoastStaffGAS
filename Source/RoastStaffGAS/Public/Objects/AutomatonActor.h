// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/PoolableInterface.h"
#include "Interface/SkillEffectInterface.h"
#include "AutomatonActor.generated.h"

class UAbilitySystemComponent;
class UGameplayEffect;
class UNiagaraSystem;
class UNiagaraComponent;
class ABaseProjectile;

/**
 * AAutomatonActor
 * 설치형 자율 발사 + 힐 터렛 Actor.
 *
 */
UCLASS()
class ROASTSTAFFGAS_API AAutomatonActor : public AActor, public IPoolableInterface, public ISkillEffectInterface
{
	GENERATED_BODY()

public:
	AAutomatonActor();
	virtual void BeginPlay() override;

	// ── IPoolableInterface ──────────────────────────────────────────────────
	virtual void OnPoolActivate() override;
	virtual void OnPoolDeactivate() override;

	// ── ISkillEffectInterface ───────────────────────────────────────────────
	virtual void InitEffect(const FSkillEffectInitData& InitData) override;

private:
	/** 가장 가까운 적 탐색 → Actor 회전 업데이트 */
	void TargetUpdateTick();
	/** 현재 Forward 기준 Spread 5발 투사체 스폰 */
	void FireTick();
	/** InstigatorASC에 힐 GE Apply */
	void HealTick();
	/** Duration 만료 → 정리 및 풀 반납 */
	void OnLifetimeExpired();
	void ReturnToPool();

protected:
	/** 적 탐색 반경 (cm) */
	UPROPERTY(EditDefaultsOnly, Category = "Automaton")
	float TargetSearchRadius = 2500.f;

	/** 스폰 즉시 재생 FX */
	UPROPERTY(EditDefaultsOnly, Category = "Automaton|FX")
	TObjectPtr<UNiagaraSystem> SpawnFX;
	/** 투사체 클래스 (LRLinearProjectile 권장) */
	UPROPERTY(EditDefaultsOnly, Category = "Automaton")
	TSubclassOf<ABaseProjectile> ProjectileClass;

	/** 투사체 데미지 배율 — BP에서 스킬별로 설정. 힐 배율(DamageMultiplier)과 별도 적용. */
	UPROPERTY(EditDefaultsOnly, Category = "Automaton")
	float ProjectileDamageMultiplier = 1.4f;

private:
	// ── 타이밍 상수 ─────────────────────────────────────────────────────────
	static constexpr float AutomatonFireInterval = 1.0f;
	static constexpr float AutomatonHealInterval = 2.0f;
	static constexpr float TargetUpdateInterval  = 0.5f;
	/** 오토마톤 발사 Spread 각도 배열 (도) */
	static constexpr int32 SpreadCount = 5;
	static constexpr float SpreadAngles[SpreadCount] = { -40.f, -20.f, 0.f, 20.f, 40.f };  // NOLINT
	/** 투사체 발사 기준 오프셋 (cm) */
	static constexpr float ProjectileSpawnOffset = 80.f;

	// ── 런타임 캐시 ─────────────────────────────────────────────────────────
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> InstigatorASC;
	/** 힐 GE 클래스 (SkillGEClass) */
	UPROPERTY()
	TSubclassOf<UGameplayEffect> HealGEClass;
	/** 투사체 데미지 GE 클래스 (StatusGEClass) */
	UPROPERTY()
	TSubclassOf<UGameplayEffect> CachedDamageGEClass;

	float CachedHealAmount   = 0.f;
	float CachedDamageAmount = 0.f;
	float CachedLifetime    = 0.f;
	float CachedProjSpeed   = 1200.f;
	int32 CachedSpawnCount  = SpreadCount;

	/** InitEffect 시 전달받은 시전자 Forward — 적 미탐지 시 폴백 방향 */
	FVector CachedForwardDir = FVector::ForwardVector;

	/** 적 미탐지 경고 중복 방지 플래그 */
	bool bNoTargetWarned = false;

	UPROPERTY()
	TObjectPtr<UNiagaraComponent> SpawnedFXComp;

	FTimerHandle TargetUpdateHandle;
	FTimerHandle FireTimerHandle;
	FTimerHandle HealTimerHandle;
	FTimerHandle LifetimeTimerHandle;
};