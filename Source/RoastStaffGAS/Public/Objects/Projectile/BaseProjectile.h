// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/RuntimeDataStructs.h"
#include "BaseProjectile.generated.h"

/**
 * ABaseProjectile
 * 모든 투사체의 베이스 클래스.
 * 일단 오브젝트 풀링 없이 Spawn/Destroy 방식 사용.
 *
 * 자식 클래스 확장 포인트:
 * - OnProjectileInitialized() : 타입별 추가 초기화 (Arc 중력, Pierce 카운트 등)
 * - OnProjectileHit()         : 타입별 충돌 처리
 * - OnProjectileExpired()     : 타입별 수명 만료 처리
 */

class USphereComponent;
class UProjectileMovementComponent;
class UGameplayEffect;

UCLASS()
class ROASTSTAFFGAS_API ABaseProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	ABaseProjectile();

protected:
	virtual void BeginPlay() override;
	// 타입별 추가 초기화 (Arc 중력, Pierce 카운트 등)
	virtual void OnProjectileInitialized() {   }
	// @return true면 베이스가 Destroy, false면 자식이 직접 처리
	virtual bool OnProjectileHit(AActor* OtherActor, const FHitResult& Hit) { return true; }
	// 수명 만료 시 자식 처리 (폭발 범위 데미지 등)
	virtual void OnProjectileExpired() {   }
	
	// 충돌 이벤트 — OnHit에서 OnProjectileHit으로 위임
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, 
		UPrimitiveComponent* OtherComp,	FVector NormalImpulse,	const FHitResult& Hit);
	
	// GE 적용 헬퍼
	void ApplyEffectToTarget(UAbilitySystemComponent* TargetASC, TSubclassOf<UGameplayEffect> EffectClass,	float DamageValue);
	
	// 수명 만료 처리
	void OnLifetimeExpired();

public:	
	// GA가 스폰 직후 호출 — final로 막아 공통 초기화 보장
	void InitProjectile(const FRSSkillInitData& InInitData);

	
protected:
	UPROPERTY(VisibleAnywhere, Category = "MY|Projectile")
	TObjectPtr<USphereComponent> SphereComp;
	UPROPERTY(VisibleAnywhere, Category = "MY|Projectile")
	TObjectPtr<UProjectileMovementComponent> ProjectileComp;

	// GA로부터 받은 초기화 데이터 
	FRSSkillInitData InitData;

private:
	FTimerHandle LifetimeTimerHandle;
	
};
