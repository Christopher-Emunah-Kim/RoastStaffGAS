// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/Attributes/EnemyAttributeSet.h"
#include "Character/Enemy/EnemyBaseCharacter.h"
#include "GameplayEffectTypes.h"
#include "GAS/Tags/RSGameplayTags.h"
#include "RoastStaffGAS.h"

UEnemyAttributeSet::UEnemyAttributeSet()
{
	// 기본값은 BaseAttributeSet 생성자에서 초기화
	// 실제값은 DT_Enemy에서 주입.
}

void UEnemyAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	// Base: CurrentHP 클램핑 + 사망 델리게이트 발행
	Super::PostGameplayEffectExecute(Data);

	// CurrentHP가 아니거나 피해가 아니면 반응 없음
	if (Data.EvaluatedData.Attribute != GetCurrentHPAttribute())
	{
		return;
	}
	if (Data.EvaluatedData.Magnitude >= 0.f || GetCurrentHP() <= 0.f)
	{
		return;
	}

	AEnemyBaseCharacter* Enemy = Cast<AEnemyBaseCharacter>(GetOwningActor());
	if (!Enemy)
	{
		KHS_WARN(TEXT("소유 Actor가 AEnemyBaseCharacter가 아님"));
		return;
	}

	// AoE 히트: GE Context HitResult Location(AoE Center) → 에너미 방향으로 밀어냄
	// 투사체 히트: Instigator(플레이어) → 에너미 방향으로 밀어냄 (fallback)
	FVector ImpactDir = FVector::ForwardVector;
	const FGameplayEffectContextHandle& EffectContext = Data.EffectSpec.GetContext();
	const FHitResult* HitResult = EffectContext.GetHitResult();
	if (HitResult && !HitResult->ImpactPoint.IsZero())
	{
		// HitResult.ImpactPoint = AoE Center (GA_CharacterSkill에서 SetHitResult로 주입)
		const FVector Delta = Enemy->GetActorLocation() - HitResult->ImpactPoint;
		if (!Delta.IsNearlyZero())
		{
			ImpactDir = Delta.GetSafeNormal2D();
		}
	}
	else if (const AActor* Instigator = EffectContext.GetInstigator())
	{
		const FVector Delta = Enemy->GetActorLocation() - Instigator->GetActorLocation();
		if (!Delta.IsNearlyZero())
		{
			ImpactDir = Delta.GetSafeNormal();
		}
	}

	// GE GrantedTags에 CC 태그가 있으면 해당 CC 적용, 없으면 일반 히트 반응
	FGameplayTagContainer GrantedTags;
	Data.EffectSpec.GetAllGrantedTags(GrantedTags);
	if (GrantedTags.HasTag(RSTags::CC_Knockdown))
	{
		Enemy->ApplyKnockdown(ImpactDir);
	}
	else
	{
		Enemy->ApplyHitReact(ImpactDir);
	}
}