// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/RuntimeDataStructs.h"
#include "Interface/PoolableInterface.h"
#include "BaseProjectile.generated.h"

/**
 * ABaseProjectile
 * 모든 투사체의 베이스 클래스.
 *
 * 자식 클래스 확장 포인트:
 * - OnProjectileInitialized() : 타입별 추가 초기화 (Arc 중력, Pierce 카운트 등)
 * - OnProjectileHit()         : 타입별 충돌 처리
 * - OnProjectileExpired()     : 타입별 수명 만료 처리
 */

class USphereComponent;
class UProjectileMovementComponent;
class UGameplayEffect;
struct FProjectileInitData;

UCLASS()
class ROASTSTAFFGAS_API ABaseProjectile : public AActor, public IPoolableInterface
{
	GENERATED_BODY()
	
public:	
	ABaseProjectile();
	
	//IPoolableInterface
	virtual void OnPoolActivate() override;
	virtual void OnPoolDeactivate() override;
	
	// GA가 스폰 직후 호출 - 공통 초기화
	void InitProjectile(const FProjectileInitData& InInitData);
	
protected:
	virtual void BeginPlay() override;
	// 타입별 추가 초기화
	virtual void OnProjectileInitialized();
	// @return true면 베이스가 GE처리하고 ReturnPool, false면 자식이 직접 처리
	virtual bool OnProjectileHit(AActor* OtherActor, const FHitResult& Hit) { return true; }
	// 수명 만료 시 자식 처리 (ARC 미착탄 폭발 등)
	virtual void OnProjectileExpired();

	// 충돌 이벤트 — Blocking (SINGLE/AREA/벽)
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,	FVector NormalImpulse,	const FHitResult& Hit);
	// 오버랩 이벤트 — PIERCE: Pawn을 Overlap으로 설정하여 관통 처리
	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

protected:
	// GE 적용 헬퍼 — 자식 클래스(HomingBounce 등) 접근 허용
	void ApplyMultipleEffectsToTarget(UAbilitySystemComponent* TargetASC, float DamageMultiplier);

private:
	//풀 반환 헬퍼
	void ReturnToPool();
	void ApplyEffectToTarget(UAbilitySystemComponent* TargetASC, TSubclassOf<UGameplayEffect> EffectClass,	float DamageValue);
	// 수명 만료 처리
	void OnLifetimeExpired();
	// 일반 공격 처리
	void HandleHitEvent(AActor* OtherActor, const FHitResult& Hit);
	// 폭발 공격 처리(AREA)
	void HandleAreaHit(const FVector& Center);
	// 관통 공격 처리(PIERCE)
	void HandlePierceHit(AActor* OtherActor);
	// 바운스 공격 처리(HOMING_BOUNCE) — 적 충돌 시 데미지 후 다음 가까운 적으로 유도
	void HandleBounceHit(AActor* OtherActor, const FHitResult& Hit);

private:
	FTimerHandle LifetimeTimerHandle;
	bool bHasExploded = false;
	bool bHasPierceFinished = false;
	int32 PierceHitCount = 0;
	int32 BounceHitCount = 0;
	TSet<TWeakObjectPtr<AActor>> PiercedActors;

	static constexpr int32 MAX_BOUNCE_COUNT = 3;
	
protected:
	UPROPERTY(VisibleAnywhere, Category = "MY|Projectile")
	TObjectPtr<USphereComponent> SphereComp;
	UPROPERTY(VisibleAnywhere, Category = "MY|Projectile")
	TObjectPtr<UProjectileMovementComponent> ProjectileComp;

	// GA로부터 받은 초기화 데이터 
	UPROPERTY()
	FProjectileInitData InitData;
	
};
