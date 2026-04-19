// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/BaseCharacter.h"
#include "Data/DataTableStructs.h"
#include "Interface/PoolableInterface.h"
#include "EnemyBaseCharacter.generated.h"


/**
 * ABaseEnemyCharacter
 *
 * - 에너미 베이스 클래스.
 * - 스폰 직후 InitializeEnemy(EnemyID)를 반드시 호출
 * - 사망 시 OnEnemyKilledDel을 Broadcast → StageWaveSubsystem이 수신.
 */

class UEnemyAttributeSet;
class UAbilitySystemComponent;
class UGameDataSubsystem;
class UWidgetComponent;
class UEnemyHPBarWidget;
class UFloatingDamageWidget;
class AEnemyProjectile;
class UGameplayEffect;
class UMaterialInstanceDynamic;

// 에너미 처치 델리게이트 — StageWaveSubsystem이 구독
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyKilled, FName, EnemyID);


UCLASS()
class ROASTSTAFFGAS_API AEnemyBaseCharacter : public ABaseCharacter, public IPoolableInterface
{
	GENERATED_BODY()

public:
	AEnemyBaseCharacter();

	// 스폰 직후 DT_Enemy 기반 스탯 주입 및 AI 시작
	UFUNCTION(BlueprintCallable, Category = "MY|Enemy")
	void InitializeEnemy(FName InEnemyID);
	void ForcePoolActive();
	
	/** 일반 피격 반응 (넉백+히트스탑+이미시브) — CC 없는 스킬에 사용 */
	void ApplyHitReact(FVector ImpactDir);

	/** 넉다운 — 포물선 날아가기 + 래그돌 + 기립 타이머 */
	void ApplyKnockdown(FVector ImpactDir);
	
	FORCEINLINE FName GetEnemyID() const { return EnemyID; }

protected:
	virtual void BeginPlay() override;

	// ABaseCharacter 오버라이드
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual UBaseAttributeSet* GetBaseAttributeSet() const override;
	virtual void InitializeAbilitySystem() override;
	virtual void HandleDeath() override;

	// IPoolableInterface 구현
	virtual void OnPoolActivate() override;
	virtual void OnPoolDeactivate() override;

	/** 투사체 발사 공통 헬퍼 — AttackGEClass/ProjectileClass 방어 체크 후 풀 획득 → InitEnemyProjectile 호출 */
	void LaunchEnemyProjectile(const FVector& Direction, float Damage);

private:
	bool ApplyStatData(FEnemyStaticData& EnemyData);
	bool StartEnemyAI(FEnemyStaticData EnemyData);

	/** WidgetComponent에 HPBarWidget을 설정하고 ASC에 바인딩 */
	void SetupHPBar();

	/** 피격 시 메시 이미시브 플래시 + 복원 타이머 설정 */
	void MaterialEmissiveFlash();

public:
	// 처치 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "MY|Enemy")
	FOnEnemyKilled OnEnemyKilledDel;


protected:
	// ── 공통 파라미터 (InitializeXxxParams에서 FEnemyExtData 기반 주입) ──
	float PreferredRange     = 400.f;
	float MaxAttackRange     = 800.f;
	float ProjectileSpeed    = 600.f;
	float ProjectileLifetime = 3.f;
	float AttackDamage       = 0.f;
	// 중복 초기화 방지
	bool bIsInitialized = false;

	// ── 투사체 공통 클래스 (BP에서 할당) ──
	UPROPERTY(EditDefaultsOnly, Category = "MY|Enemy|Projectile")
	TSubclassOf<UGameplayEffect> AttackGEClass;
	UPROPERTY(EditDefaultsOnly, Category = "MY|Enemy|Projectile")
	TSubclassOf<AEnemyProjectile> ProjectileClass;

private:
	// ASC
	UPROPERTY(VisibleAnywhere, Category = "MY|GAS")
	TObjectPtr<UAbilitySystemComponent> ASC;
	// 에너미 전용 AS
	UPROPERTY(VisibleAnywhere, Category = "MY|GAS")
	TObjectPtr<UEnemyAttributeSet> EnemyAttributeSet;

	// UI 컴포넌트
	UPROPERTY(VisibleAnywhere, Category = "MY|UI")
	TObjectPtr<UWidgetComponent> HPBarWidgetComp;
	UPROPERTY(EditDefaultsOnly, Category = "MY|UI")
	TSubclassOf<UEnemyHPBarWidget> HPBarWidgetClass;

	// EnemyID 캐싱(초기화/사망 시 델리게이트 용도)
	FName EnemyID;

	/** 사망 후 풀 반납까지 대기 시간 (초) — 사망 연출 길이에 맞춰 BP에서 조정 */
	UPROPERTY(EditDefaultsOnly, Category = "MY|Enemy")
	float DeathPoolReturnDelay = 2.f;

	// ── 피격 반응 설정 (BP에서 에너미 종류별 조정 가능) ──
	UPROPERTY(EditDefaultsOnly, Category = "MY|Enemy|HitReact")
	float KnockbackForce = 400.f;
	UPROPERTY(EditDefaultsOnly, Category = "MY|Enemy|HitReact")
	float HitstopDuration = 0.08f;
	UPROPERTY(EditDefaultsOnly, Category = "MY|Enemy|HitReact")
	float FlashIntensity = 3.f;
	UPROPERTY(EditDefaultsOnly, Category = "MY|Enemy|HitReact")
	float FlashDuration = 0.12f;

	// ── 넉다운 설정 ──
	/** 넉다운 몽타주 — 쓰러지기 + 기립 섹션 포함. BP에서 에너미별 할당 */
	UPROPERTY(EditDefaultsOnly, Category = "MY|Enemy|Knockdown")
	TObjectPtr<UAnimMontage> KnockdownMontage;
	/** 넉다운 수평 이동 속도 */
	UPROPERTY(EditDefaultsOnly, Category = "MY|Enemy|Knockdown")
	float KnockdownLaunchForce = 300.f;
	/** 넉다운 수직 속도 (포물선 높이) */
	UPROPERTY(EditDefaultsOnly, Category = "MY|Enemy|Knockdown")
	float KnockdownLaunchZ = 400.f;
	/** 몽타주 재생 시간 — 이 시간 후 AI 재개 (몽타주 길이에 맞춰 BP에서 조정) */
	UPROPERTY(EditDefaultsOnly, Category = "MY|Enemy|Knockdown")
	float KnockdownDuration = 2.0f;

	/** 피격 이미시브 플래시에 사용되는 동적 머티리얼 인스턴스 캐시 */
	UPROPERTY()
	TArray<TObjectPtr<UMaterialInstanceDynamic>> CachedMIDs;
	/** 사망 후 풀 반납 타이머 */
	FTimerHandle DeathReturnTimerHandle;
	/** 히트스탑 복원 타이머 */
	FTimerHandle HitstopTimerHandle;
	/** 이미시브 플래시 복원 타이머 */
	FTimerHandle FlashTimerHandle;
	/** 넉다운 기립 타이머 */
	FTimerHandle KnockdownRecoverTimerHandle;
};
