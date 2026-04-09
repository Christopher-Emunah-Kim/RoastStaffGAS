// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Attributes/BaseAttributeSet.h"
#include "RoastStaffGAS.h"

UBaseAttributeSet::UBaseAttributeSet()
{
	// 기본값 실제 수치는 GDS 조회 후 GE로 주입
	InitMaxHP(0.f);
	InitCurrentHP(0.f);
	InitMoveSpeed(0.f);
}

void UBaseAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	// CurrentHP를 [0, MaxHP] 범위로 클램핑
	if (Attribute == GetCurrentHPAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHP());
	}

	// MaxHP 감소 시 CurrentHP가 초과하지 않도록 클램핑
	if (Attribute == GetMaxHPAttribute())
	{
		SetCurrentHP(FMath::Min(GetCurrentHP(), NewValue));
	}

	// MoveSpeed 하한 고정 — 0 이하로 내려가지 않도록
	if (Attribute == GetMoveSpeedAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.f);
	}
}

void UBaseAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetCurrentHPAttribute())
	{
		// 클램핑 재보정 (PostExecute 시점에도 한 번 더)
		SetCurrentHP(FMath::Clamp(GetCurrentHP(), 0.f, GetMaxHP()));

		KHS_INFO(TEXT("[HP 변경] %s → CurrentHP: %.0f / MaxHP: %.0f"),
		*GetOwningActor()->GetName(), GetCurrentHP(), GetMaxHP());
		
		// HP 변경 이벤트 발행 — UI 갱신용
		OnHealthChangedDel.Broadcast(GetCurrentHP(), GetMaxHP());

		// 사망 판정 이벤트 발행
		if (GetCurrentHP() <= 0.f)
		{
			OnDeathDel.Broadcast();
		}
	}
}