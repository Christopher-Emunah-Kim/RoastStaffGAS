// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/PoolableInterface.h"
#include "Interface/SkillEffectInterface.h"
#include "PullVortexActor.generated.h"

class UAbilitySystemComponent;
class UGameplayEffect;

/**
 * APullVortexActor
 * 흡입 + 다단 데미지 + 넉다운 장판 Actor.
 *
 * InitEffect() 호출 시:
 *   - PullTick: PullRadius 내 적을 중심으로 흡입 (PullTickRate 간격)
 *   - HitTick:  EffectRadius 내 적에게 SkillGEClass 적용 (HitInterval 간격, HitCount 회)
 *   - 마지막 HitTick: StatusGEClass (넉다운) 추가 적용
 *   - Duration 후 ReturnToPool
 *
 * EditDefaultsOnly 파라미터는 BP에서 스킬별로 설정.
 */
UCLASS()
class ROASTSTAFFGAS_API APullVortexActor : public AActor, public IPoolableInterface, public ISkillEffectInterface
{
	GENERATED_BODY()

public:
	APullVortexActor();
	virtual void BeginPlay() override;

	// ── IPoolableInterface ──────────────────────────────────────────────────
	virtual void OnPoolActivate() override;
	virtual void OnPoolDeactivate() override;

	// ── ISkillEffectInterface ───────────────────────────────────────────────
	virtual void InitEffect(const FSkillEffectInitData& InitData) override;


private:
	/** 주기적으로 PullRadius 내 적을 중심으로 흡입. */
	void PullTick();

	/**
	 * EffectRadius 내 적에게 SkillGEClass 적용.
	 * 마지막 히트(RemainingHitCount == 1)에서 StatusGEClass 추가 적용 후 타이머 정지.
	 */
	void HitTick();

	void ApplyGEToTarget(AActor* TargetActor, TSubclassOf<UGameplayEffect> GEClass);
	void ReturnToPool();

protected:
	/** 흡입이 작용하는 범위 (cm). EffectRadius보다 크게 설정 권장. */
	UPROPERTY(EditDefaultsOnly, Category = "PullVortex")
	float PullRadius = 600.f;
	/** 흡입 틱당 적용하는 충격량 (cm/s). */
	UPROPERTY(EditDefaultsOnly, Category = "PullVortex")
	float PullStrength = 500.f;
	/** 흡입 틱 간격 (초). */
	UPROPERTY(EditDefaultsOnly, Category = "PullVortex")
	float PullTickRate = 0.15f;
	/** 총 데미지 히트 횟수. */
	UPROPERTY(EditDefaultsOnly, Category = "PullVortex")
	int32 HitCount = 5;
	/** 데미지 히트 간격 (초). */
	UPROPERTY(EditDefaultsOnly, Category = "PullVortex")
	float HitInterval = 0.5f;
	
private:
	// ── 런타임 캐시 ─────────────────────────────────────────────────────────
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> CachedInstigatorASC;
	UPROPERTY()
	TSubclassOf<UGameplayEffect> CachedSkillGEClass;
	UPROPERTY()
	TSubclassOf<UGameplayEffect> CachedStatusGEClass;

	float CachedAmount       = 0.f;
	float CachedEffectRadius = 0.f;

	int32 RemainingHitCount  = 0;

	FTimerHandle PullTimerHandle;
	FTimerHandle HitTimerHandle;
	FTimerHandle DurationTimerHandle;
};
