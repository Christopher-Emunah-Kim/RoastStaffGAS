// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Enemy/EnemyBaseCharacter.h"
#include "Data/DataTableStructs.h"
#include "RangedEnemy.generated.h"

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
	/** 투사체 발사 몽타주 — BP에서 할당. 미할당 시 생략 */
	UPROPERTY(EditDefaultsOnly, Category = "MY|Enemy|Attack")
	TObjectPtr<UAnimMontage> AttackMontage;
};
