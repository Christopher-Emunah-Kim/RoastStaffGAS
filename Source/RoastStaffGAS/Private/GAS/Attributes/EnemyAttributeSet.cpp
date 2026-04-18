// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/Attributes/EnemyAttributeSet.h"
#include "Character/Enemy/EnemyBaseCharacter.h"
#include "GameplayEffectTypes.h"
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

	// 공격자 → 에너미 방향으로 넉백 벡터 계산
	FVector ImpactDir = FVector::ForwardVector;
	const FGameplayEffectContextHandle& EffectContext = Data.EffectSpec.GetContext();
	if (const AActor* Instigator = EffectContext.GetInstigator())
	{
		const FVector Delta = Enemy->GetActorLocation() - Instigator->GetActorLocation();
		if (!Delta.IsNearlyZero())
		{
			ImpactDir = Delta.GetSafeNormal();
		}
	}

	Enemy->ApplyHitReact(ImpactDir);
}