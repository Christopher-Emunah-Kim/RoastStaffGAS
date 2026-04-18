// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/PoolableInterface.h"
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
 * PoolingSubsystem에서 스폰 → InitGroundEffect()로 초기화 → Duration 후 자동 ReturnToPool.
 * OnPoolActivate: OverlapSphere 활성화 + FX 스폰.
 * OnPoolDeactivate: 충돌 비활성화 + FX 중단 + 타이머 클리어.
 *
 * Niagara FX에는 "Radius"(float), "ElementColor"(LinearColor) 파라미터 
 */
UCLASS()
class ROASTSTAFFGAS_API AGroundEffectActor : public AActor, public IPoolableInterface
{
	GENERATED_BODY()

public:
	AGroundEffectActor();
	virtual void BeginPlay() override;

	// ── IPoolableInterface ──────────────────────────────────────────────────
	virtual void OnPoolActivate() override;
	virtual void OnPoolDeactivate() override;

	/**
	 * GA_CharacterSkill::ExecuteGroundEffect에서 스폰 직후 호출.
	 * 모든 런타임 파라미터를 캐시하고 이펙트를 활성화.
	 *
	 * @param InstigatorASC  시전자 ASC (GE 시전 주체)
	 * @param Duration       장판 지속 시간 (초). 0이면 무한 지속
	 * @param OverlapGEClass Overlap 시 적에게 적용할 GE 클래스
	 * @param Radius         Overlap 감지 반경 (cm)
	 * @param FXClass        부착할 Niagara FX (Soft Reference)
	 * @param Amount         SetByCaller 데미지 수치 (Data.WeaponBaseDamage 주입)
	 */
	void InitGroundEffect(
		UAbilitySystemComponent* InstigatorASC,
		float Duration,
		TSubclassOf<UGameplayEffect> OverlapGEClass,
		float Radius,
		TSoftObjectPtr<UNiagaraSystem> FXClass,
		float Amount);

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
