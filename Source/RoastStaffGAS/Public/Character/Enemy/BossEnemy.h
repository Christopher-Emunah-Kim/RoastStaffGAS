// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Enemy/EnemyBaseCharacter.h"
#include "Data/DataTableStructs.h"
#include "BossEnemy.generated.h"

struct FOnAttributeChangeData;
class AEnemyProjectile;
class UGameplayEffect;
class UNiagaraSystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBossKilled);

/**
 * ABossEnemy
 *
 * - Phase1: Chase + Shockwave (범위 근접)
 * - Phase2: HP ≤ Phase2HPRatio 도달 시 1회 전환 연출 후 활성.
 *           Chase → 8방향 투사체(중거리) or Shockwave(근접 범위 우선)
 * - 페이즈 전환: PauseAI → Montage + FX → ActivatePhase2 → ResumeAI + BB bIsPhase2=true
 * - EnemySpawner가 OnBossKilledDel 구독 → 보스 HUD 해제 담당.
 */
UCLASS()
class ROASTSTAFFGAS_API ABossEnemy : public AEnemyBaseCharacter
{
	GENERATED_BODY()

public:
	ABossEnemy();

	/** EnemySpawner가 InitializeEnemy 이후 호출 */
	void InitializeBossParams(float InAttackDamage, const FEnemyExtData& ExtData);
	/** BTTask_ExecuteShockwave가 PrepareTime 대기 후 호출 */
	UFUNCTION(BlueprintCallable, Category = "MY|Enemy|Boss")
	void ExecuteShockwave();
	/** BTTask_FireSpreadProjectile가 Phase2에서 호출 */
	UFUNCTION(BlueprintCallable, Category = "MY|Enemy|Boss")
	void FireSpreadProjectile();
	/** BTDecorator_ShockwaveReady가 쿨타임 만료 여부를 조회 */
	bool IsShockwaveReady() const;
	/** BTTask_ExecuteShockwave가 완료 후 호출 — 쿨타임 시작 */
	void MarkShockwaveUsed();
	
	FORCEINLINE bool  IsPhase2()             const { return bPhaseTransitioned; }
	FORCEINLINE float GetPhase2HPRatio()     const { return Phase2HPRatio; }
	FORCEINLINE float GetShockwaveRadius()   const { return ShockwaveRadius; }
	FORCEINLINE float GetShockwaveCooldown() const { return ShockwaveCooldown; }
	FORCEINLINE float GetShockwavePrepareTime() const { return ShockwavePrepareTime; }
	FORCEINLINE float GetPreferredRange()    const { return PreferredRange; }
	FORCEINLINE float GetMaxAttackRange()    const { return MaxAttackRange; }

protected:
	virtual void BeginPlay() override;
	virtual void HandleDeath() override;

private:
	void OnHPChanged(const FOnAttributeChangeData& Data);

	void CheckPhaseTransition(float NewHP, float MaxHP);
	void StartPhaseTransition();
	void ActivatePhase2();

	void ApplyShockwaveDamage(AActor* Target);

public:
	/** EnemySpawner가 구독 — 보스 사망 시 HUD 해제 트리거 */
	UPROPERTY(BlueprintAssignable, Category = "MY|Enemy|Boss")
	FOnBossKilled OnBossKilledDel;

private:
	// ── Shockwave 파라미터 ───────────────────────────────────────────────
	float ShockwaveRadius      = 300.f;
	float ShockwaveDamage      = 0.f;
	float ShockwaveCooldown    = 8.f;
	float ShockwavePrepareTime = 1.5f;

	// ── Phase2 파라미터 ──────────────────────────────────────────────────
	float Phase2HPRatio       = 0.5f;
	float Phase2MoveSpeedMult = 1.f;
	float Phase2DamageMult    = 1.f;
	bool  bPhaseTransitioned  = false;
	/** 마지막 Shockwave 사용 시각 — 초기값 -999로 설정해 게임 시작 즉시 준비 상태 */
	float LastShockwaveTime   = -999.f;

	/** 전환 연출용 에셋 — InitializeBossParams에서 동기 로드 */
	UPROPERTY()
	TObjectPtr<UNiagaraSystem> LoadedTransitionFX;
	UPROPERTY()
	TObjectPtr<UAnimMontage> LoadedTransitionMontage;

	// ── 공격 몽타주 ─────────────────────────────────────────────────────
	/** Shockwave 발동 몽타주 — BP에서 할당. 미할당 시 생략 */
	UPROPERTY(EditDefaultsOnly, Category = "MY|Enemy|Attack")
	TObjectPtr<UAnimMontage> ShockwaveMontage;
	/** Phase2 방사형 투사체 발사 몽타주 — BP에서 할당. 미할당 시 생략 */
	UPROPERTY(EditDefaultsOnly, Category = "MY|Enemy|Attack")
	TObjectPtr<UAnimMontage> SpreadProjectileMontage;

	/** 연출 없을 때 사용할 기본 전환 대기 시간 */
	UPROPERTY(EditDefaultsOnly, Category = "MY|Enemy|Boss")
	float DefaultTransitionDuration = 2.f;

	FTimerHandle PhaseTransitionTimerHandle;

	// ── GE / 투사체 클래스 (BP 할당) ─────────────────────────────────────
	/** Shockwave 범위 데미지 GE */
	UPROPERTY(EditDefaultsOnly, Category = "MY|Enemy|Boss")
	TSubclassOf<UGameplayEffect> ShockwaveGEClass;
};
