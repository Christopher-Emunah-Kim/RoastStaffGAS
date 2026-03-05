// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/EnemyBaseCharacter.h"
#include "MeleeEnemy.generated.h"

/**
 * AMeleeEnemy
 *
 * - 근접 공격 에너미.
 */

class USphereComponent;
class UGameplayEffect;

UCLASS()
class ROASTSTAFFGAS_API AMeleeEnemy : public AEnemyBaseCharacter
{
	GENERATED_BODY()
	
public:
	AMeleeEnemy();

protected:
	virtual void BeginPlay() override;

private:
	// BeginOverlap 콜백
	UFUNCTION()
	void OnAttackRangeBeginOverlap(UPrimitiveComponent* OverlappedComp,	AActor* OtherActor,	UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep,const FHitResult& SweepResult);
	// EndOverlap 콜백 
	UFUNCTION()
	void OnAttackRangeEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	// 공격 실행
	void TryAttack(AActor* Target);

public:
	// MeleeEnemy 전용 초기화
	void InitializeMeleeParams(float InAttackDamage, float InAttackCooldown, float InAttackRange);
	
private:
	// 공격 범위 콜라이더
	UPROPERTY(VisibleAnywhere, Category = "MY|Enemy|Attack")
	TObjectPtr<USphereComponent> AttackRangeSphere;

	// 플레이어에게 적용할 데미지 GE
	UPROPERTY(EditDefaultsOnly, Category = "MY|Enemy|Attack")
	TSubclassOf<UGameplayEffect> AttackGEClass;

	// DT_Enemy에서 주입받는 공격 파라미터
	float AttackDamage = 0.f;
	float AttackCooldown = 1.f;

	// 현재 공격 대상 플레이어(범위 내)
	TWeakObjectPtr<AActor> AttackTarget;
	// 공격 타이머
	FTimerHandle AttackRepeatTimerHandle;
	
};
