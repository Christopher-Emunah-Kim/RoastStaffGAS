// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/PoolableInterface.h"
#include "EnemyProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UGameplayEffect;
class UAbilitySystemComponent;

/**
 * AEnemyProjectile
 *
 * 에너미 전용 투사체. AActor + IPoolableInterface 직접 구현.
 * - ABaseProjectile 미상속: 플레이어 GA 기반 FProjectileInitData 의존 회피
 * - 발사: RangedEnemy / BossEnemy(Phase2)가 PoolingSubsystem에서 획득 후 InitEnemyProjectile 호출
 * - 피격: Team_Player 태그 보유 액터 ASC에 AttackGEClass 직접 Apply → ReturnToPool
 * - 수명 만료 시 ReturnToPool
 */
UCLASS()
class ROASTSTAFFGAS_API AEnemyProjectile : public AActor, public IPoolableInterface
{
	GENERATED_BODY()

public:
	AEnemyProjectile();

	// IPoolableInterface
	virtual void OnPoolActivate() override;
	virtual void OnPoolDeactivate() override;

	/**
	 * 발사 직전 호출 — 방향/속도/수명/데미지 주입
	 */
	void InitEnemyProjectile(const FVector& Direction, float Speed,	float Lifetime,	float Damage,
		TSubclassOf<UGameplayEffect> InDamageGEClass,	UAbilitySystemComponent* InSourceASC);

protected:
	virtual void BeginPlay() override;

	/** 벽/지형(WorldStatic·WorldDynamic) Block 충돌 — ReturnToPool */
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	/** Pawn Overlap — Team_Player면 데미지 후 ReturnToPool, 에너미는 통과 */
	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	void ApplyDamageToTarget(UAbilitySystemComponent* TargetASC);
	void ReturnToPool();
	void OnLifetimeExpired();

private:
	FTimerHandle LifetimeTimerHandle;
	float CachedDamage = 0.f;
	TSubclassOf<UGameplayEffect> CachedDamageGEClass;

	/** 발사한 에너미의 ASC — GE 스펙 소스로 사용 */
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> SourceASC;

protected:
	UPROPERTY(VisibleAnywhere, Category = "MY|Projectile")
	TObjectPtr<USphereComponent> SphereComp;
	UPROPERTY(VisibleAnywhere, Category = "MY|Projectile")
	TObjectPtr<UProjectileMovementComponent> ProjectileComp;
	
};
