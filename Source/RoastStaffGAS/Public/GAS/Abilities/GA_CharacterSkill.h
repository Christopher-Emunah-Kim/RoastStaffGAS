// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/GA_Base.h"
#include "Data/RuntimeDataStructs.h"
#include "GA_CharacterSkill.generated.h"

class UNiagaraSystem;

/**
 * UGA_CharacterSkill
 * 캐릭터 고유 스킬 GA. InstantAoE / SelfBuff / SpawnPreview(확정 후 발동) 처리.
 */
UCLASS()
class ROASTSTAFFGAS_API UGA_CharacterSkill : public UGA_Base
{
	GENERATED_BODY()

protected:
	virtual void OnAbilityActivated(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) override;

private:
	/** InstantAoE: 시전자 위치 기준 구형 범위 내 적에게 GE 적용 */
	void ExecuteInstantAoE(
		const FCharacterSkillExecData& ExecData,
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo);

	/** SelfBuff: 시전자 자신에게 지속시간 GE 적용 */
	void ExecuteSelfBuff(
		const FCharacterSkillExecData& ExecData,
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo);

	/** SpawnPreview: SkillManagerSubsystem이 저장한 PendingTargetLocation 기준 AoE 적용 */
	void ExecuteSpawnPreview(
		const FCharacterSkillExecData& ExecData,
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo);

	/** FXClass를 Location에 스폰 후 Radius 파라미터 주입 */
	void SpawnSkillFX(TSoftObjectPtr<UNiagaraSystem> FXClass, FVector Location, float Radius);

protected:
	/**
	 * 스킬 효과 GE 클래스 — BP에서 슬롯에 맞게 설정 필수.
	 *   InstantAoE / SpawnPreview: Data.WeaponBaseDamage SetByCaller → GE_Damage(ExecCalc) 권장
	 *   SelfBuff: 버프 속성 GE (Duration = Instant 이외)
	 */
	UPROPERTY(EditDefaultsOnly, Category = "MY|CharacterSkill")
	TSubclassOf<UGameplayEffect> SkillGEClass;
	
	static constexpr float DESTROY_FX_DELAY = 2.0f;
private:
	
};
