// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Enemy/EnemyBaseCharacter.h"
#include "Data/DataTableStructs.h"
#include "EliteEnemy.generated.h"

class USphereComponent;
class UGameplayEffect;

/**
 * AEliteEnemy
 *
 * - 원거리 투사체(기본) + 확률적 근접 돌진 에너미.
 */
UCLASS()
class ROASTSTAFFGAS_API AEliteEnemy : public AEnemyBaseCharacter
{
	GENERATED_BODY()

public:
	AEliteEnemy();

	/** EnemySpawner가 InitializeEnemy 이후 호출 */
	void InitializeEliteParams(float InAttackDamage, const FEnemyExtData& ExtData);

	/** BT 기본 루프 — 원거리 투사체 발사 */
	UFUNCTION(BlueprintCallable, Category = "MY|Enemy|Elite")
	void FireProjectile();

	/**
	 * BT 확률 분기 — 속도 부스트 + 근접 피해 감지 활성화.
	 * BTTask_MeleeCharge가 호출하고, 완료(피격 or 타임아웃) 시 EndCharge() 호출.
	 */
	UFUNCTION(BlueprintCallable, Category = "MY|Enemy|Elite")
	void MeleeCharge();

	/** BTTask_MeleeCharge가 완료(타임아웃/완료) 시 호출 — 상태 초기화 */
	void EndCharge();

	FORCEINLINE float GetPreferredRange()  const { return PreferredRange; }
	FORCEINLINE float GetMaxAttackRange()  const { return MaxAttackRange; }
	FORCEINLINE bool  IsCharging()         const { return bIsCharging; }

protected:
	virtual void BeginPlay() override;
	virtual void HandleDeath() override;

private:
	UFUNCTION()
	void OnChargeHitBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void ApplyChargeDamage(AActor* Target);

private:
	// ── 공격 몽타주 ─────────────────────────────────────────────────────
	/** 투사체 발사 몽타주 — BP에서 할당. 미할당 시 생략 */
	UPROPERTY(EditDefaultsOnly, Category = "MY|Enemy|Attack")
	TObjectPtr<UAnimMontage> ProjectileMontage;
	/** 근접 돌진 몽타주 — BP에서 할당. 미할당 시 생략 */
	UPROPERTY(EditDefaultsOnly, Category = "MY|Enemy|Attack")
	TObjectPtr<UAnimMontage> ChargeMontage;

	// ── 근접 돌진 파라미터 ────────────────────────────────────────────────
	/** 돌진 시 속도 배율 — BP에서 조정 */
	UPROPERTY(EditDefaultsOnly, Category = "MY|Enemy|Elite")
	float ChargeDamageMult = 2.f;
	/** 돌진 시 이동 속도 배율 — BP에서 조정 */
	UPROPERTY(EditDefaultsOnly, Category = "MY|Enemy|Elite")
	float ChargeSpeedMult = 2.f;

	float BaseWalkSpeed = 0.f;
	bool  bIsCharging   = false;

	/** 돌진 피해 감지 구체 */
	UPROPERTY(VisibleAnywhere, Category = "MY|Enemy|Elite")
	TObjectPtr<USphereComponent> ChargeHitSphere;

	// ── GE / 투사체 클래스 (BP 할당) ─────────────────────────────────────
	/** 근접 돌진 데미지 GE */
	UPROPERTY(EditDefaultsOnly, Category = "MY|Enemy|Elite")
	TSubclassOf<UGameplayEffect> ChargeGEClass;
};
