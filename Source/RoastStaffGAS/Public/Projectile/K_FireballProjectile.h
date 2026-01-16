// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "K_FireballProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UAbilitySystemComponent;
class UGameplayEffect;
struct FGameplayTag;

/*
 * Fireball 발사체
 * 
 * 충돌 시 처리흐름
 * - OnSphereOverlap 호출
 * - 타겟 유효 확인(자신 제외, ASC 보유 확인)
 * - 타겟이 Burning 상태인지 체크
 * - 최종 데미지 계산
 * - GE_FirebaillImpactDamage 적용(SetByCaller)
 * - GE_Burn 적용
 * - GE_BurnDamageCoolTime 적용
 * - 발사체 파괴
 */

UCLASS()
class ROASTSTAFFGAS_API AK_FireballProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AK_FireballProjectile();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
public:
	//데미지 정보 세팅. GA_Firball에서 스폰 후 호출
	UFUNCTION(BlueprintCallable, Category = "AM|Damage")
	void SetDamageInfo(float InBaseDamage, UAbilitySystemComponent* InOwnerASC);
	
	//오버랩이벤트 처리
	UFUNCTION()
	void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
		bool bFromSweep, const FHitResult& SweepResult);
	
	//타겟에게 데미지/화상효과
	//@param TargetActor, TargetASC
	void ApplyDamageAndBurn(AActor* TargetActor, UAbilitySystemComponent* TargetASC);
	
	//타겟에게 GE 적용
	//@param TargetASC 대상 ASC
	//@param EffectClass 적용 GE클래스
	//@param SetByCallerTag 태그(없으면 빈태그)
	//@param Magnitude SetByCaller값(태그가 유효할때만)
	//@return 적용 성공여부
	bool ApplyGameplayEffectToTarget(UAbilitySystemComponent* TargetASC, TSubclassOf<UGameplayEffect> EffectClass,
		FGameplayTag SetByCallerTag = FGameplayTag::EmptyTag, float Magnitude = 0.f);
	

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AM|Comp")
	TObjectPtr<USphereComponent> SphereComp;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AM|Comp")
	TObjectPtr<UProjectileMovementComponent> ProjectileComp;
	
	//GE Class
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AM|GAS|Effects")
	TSubclassOf<UGameplayEffect> ImpactDamageEffect;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AM|GAS|Effects")
	TSubclassOf<UGameplayEffect> BurnEffect;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AM|GAS|Effects")
	TSubclassOf<UGameplayEffect> BurnDamageOverTimeEffect;
	
	//Damage Info
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AM|Damage")
	float BaseDamage;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AM|Damage")
	TObjectPtr<UAbilitySystemComponent> OwnerASC;
	
	//Projectile Info
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AM|Projectile")
	float LifeSpan;
	
};
