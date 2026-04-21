// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/PoolableInterface.h"
#include "Interface/SkillEffectInterface.h"
#include "GroundEffectActor.generated.h"

class USphereComponent;
class UNiagaraSystem;
class UNiagaraComponent;
class UAbilitySystemComponent;
class UGameplayEffect;

/**
 * AGroundEffectActor
 * 장판(Ground AoE) 공통 베이스 클래스.
 *
 * PoolingSubsystem에서 스폰 → InitEffect()로 초기화 → Duration 후 자동 ReturnToPool.
 * OnPoolActivate: OverlapSphere 활성화 + FX 스폰.
 * OnPoolDeactivate: 충돌 비활성화 + FX 중단 + 타이머 클리어.
 *
 * Niagara FX에는 "Radius"(float), "ElementColor"(LinearColor) 파라미터 
 */
UCLASS()
class ROASTSTAFFGAS_API AGroundEffectActor : public AActor, public IPoolableInterface, public ISkillEffectInterface
{
	GENERATED_BODY()

public:
	AGroundEffectActor();
	virtual void BeginPlay() override;

	// ── IPoolableInterface ──────────────────────────────────────────────────
	virtual void OnPoolActivate() override;
	virtual void OnPoolDeactivate() override;

	// ── ISkillEffectInterface ───────────────────────────────────────────────
	/** 스폰 직후 호출. FSkillEffectInitData로 모든 런타임 파라미터를 수신해 이펙트 활성화. */
	virtual void InitEffect(const FSkillEffectInitData& InitData) override;

private:
	UFUNCTION()
	void OnSphereBeginOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	void ApplyGEToTarget(AActor* TargetActor);
	void ReturnToPool();

	// ── 컴포넌트 ────────────────────────────────────────────────────────────
	UPROPERTY(VisibleAnywhere, Category = "GroundEffect")
	TObjectPtr<USphereComponent> OverlapSphere;

	// ── 런타임 캐시 ─────────────────────────────────────────────────────────
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> CachedInstigatorASC;
	UPROPERTY()
	TSubclassOf<UGameplayEffect> CachedOverlapGEClass;
	UPROPERTY()
	TObjectPtr<UNiagaraComponent> SpawnedFXComp;

	float CachedAmount = 0.f;
	FTimerHandle DurationTimerHandle;
};
