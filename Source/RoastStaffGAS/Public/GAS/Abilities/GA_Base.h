// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "System/LoggingSystem.h"
#include "GA_Base.generated.h"


/**
 * UGA_Base
 * 프로젝트 내 모든 GA의 베이스 클래스.
 *
 * - ActivateAbility를 final로 막아 공통 캐싱을 보장
 * - 자식 GA는 OnAbilityActivated만 오버라이딩
 * - SpawnProjectiles 공통 헬퍼 제공
 */

class ABaseCharacter;
class ABaseProjectile;
struct FRSSkillInitData;

UCLASS()
class ROASTSTAFFGAS_API UGA_Base : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGA_Base();

protected:
	// ActivateAbility를 final로 잠금 
	// (GAS 이벤트 처리 위해 공통 캐싱 후 OnAbilityActivated 호출)
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override final;

	// 자식 GA가 오버라이딩 사용
	virtual void OnAbilityActivated(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo);

	// SpawnData 기반 투사체 스폰 헬퍼
	void SpawnProjectiles(TSubclassOf<ABaseProjectile> ProjectileClass, const FRSSkillInitData& InitData);

	// GA 오너의 ASC 반환
	UAbilitySystemComponent* GetOwnerASC() const;

	template<typename T>
	bool LoadRequiredClass(const TSoftClassPtr<T>& SoftPtr, TSubclassOf<T>& OutClass, const FName& ContextID);
	template<typename T>
	TSubclassOf<T> LoadOptionalClass(const TSoftClassPtr<T>& SoftPtr, const FName& ContextID) const;
	
protected:
	const float SPAWN_OFFSET = 200.f;
	
	// 이벤트 발생자 - ActivateAbility에서 캐싱
	UPROPERTY()
	TObjectPtr<const ABaseCharacter> CachedInstigator;
	
};

template<typename T>
bool UGA_Base::LoadRequiredClass(const TSoftClassPtr<T>& SoftPtr, TSubclassOf<T>& OutClass, const FName& ContextID)
{
	OutClass = SoftPtr.LoadSynchronous();
	return ensureMsgf(OutClass,	TEXT("필수 클래스 로드 실패 — ID: %s / Class: %s"),	*ContextID.ToString(), *SoftPtr.ToString());
}


template<typename T>
TSubclassOf<T> UGA_Base::LoadOptionalClass(const TSoftClassPtr<T>& SoftPtr, const FName& ContextID) const
{
	if (SoftPtr.IsNull())
	{
		return nullptr;
	}

	TSubclassOf<T> Result = SoftPtr.LoadSynchronous();
	if (!Result)
	{
		KHS_WARN(TEXT("선택 클래스 로드 실패 — 스킵. ID: %s / Class: %s"),	*ContextID.ToString(),	*SoftPtr.ToString());
	}

	return Result;
}