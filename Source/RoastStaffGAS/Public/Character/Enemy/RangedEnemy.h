// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Enemy/EnemyBaseCharacter.h"
#include "Data/DataTableStructs.h"
#include "RangedEnemy.generated.h"

class AEnemyProjectile;
class UGameplayEffect;

/**
 * ARangedEnemy
 *
 * - 원거리 투사체 에너미.
 */
UCLASS()
class ROASTSTAFFGAS_API ARangedEnemy : public AEnemyBaseCharacter
{
	GENERATED_BODY()

public:
	ARangedEnemy();

	/** EnemySpawner가 InitializeEnemy 이후 호출 — GDS에서 StaticData·ExtData 조회 후 주입 */
	void InitializeRangedParams(float InAttackDamage, const FEnemyExtData& ExtData);

	/** BTTask_FireProjectile에서 호출 — 플레이어 방향으로 투사체 1발 발사 */
	UFUNCTION(BlueprintCallable, Category = "MY|Enemy|Ranged")
	void FireProjectile();

	FORCEINLINE float GetPreferredRange()  const { return PreferredRange; }
	FORCEINLINE float GetMaxAttackRange()  const { return MaxAttackRange; }

private:
	/** PoolingSubsystem에서 EnemyProjectile 획득 후 초기화 */
	void LaunchProjectile(const FVector& Direction);

private:
	float PreferredRange  = 400.f;
	float MaxAttackRange  = 800.f;
	float ProjectileSpeed = 600.f;
	float ProjectileLifetime = 3.f;
	float AttackDamage    = 0.f;

	/** 플레이어 ASC에 적용할 데미지 GE — BP에서 할당 */
	UPROPERTY(EditDefaultsOnly, Category = "MY|Enemy|Ranged")
	TSubclassOf<UGameplayEffect> AttackGEClass;
	/** 투사체 클래스 — BP에서 할당 (BP_EnemyProjectile) */
	UPROPERTY(EditDefaultsOnly, Category = "MY|Enemy|Ranged")
	TSubclassOf<AEnemyProjectile> ProjectileClass;
};
