// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/GA_Base.h"
#include "Objects/Data/RSSkillData.h"
#include "GA_ProjectileAttack.generated.h"

/**
 * UGA_ProjectileAttack
 * 투사체형 스킬의 범용 GA.
 * Fireball, Iceball, DarkPierce 등 모든 투사체형 스킬이 이 GA를 공유
 *
 * SkillID는 GiveAbility 시점에 FGameplayAbilitySpec.SourceObject(URSSkillData)로 주입
 * GA는 활성화 시 SourceObject에서 SkillID를 꺼내 GDS를 조회
 */
UCLASS()
class ROASTSTAFFGAS_API UGA_ProjectileAttack : public UGA_Base
{
	GENERATED_BODY()
	
public:
	UGA_ProjectileAttack();

private:
	//투사체 생성 헬퍼
	bool PrepareProjectileData(const URSSkillData* SkillData, TSubclassOf<ABaseProjectile>& OutClass,  FRSSkillInitData& OutInitData);
	
protected:
	virtual void OnAbilityActivated(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) override;
	
	
};
