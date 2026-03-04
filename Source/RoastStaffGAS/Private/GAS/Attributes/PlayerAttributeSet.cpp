// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Attributes/PlayerAttributeSet.h"
#include "RoastStaffGAS.h"

UPlayerAttributeSet::UPlayerAttributeSet()
{
	//기본값 초기화 - 실제 사용시엔 로드해서 넣기
	InitATK(0.f);
	InitDEF(0.f);
	InitAttackSpeed(0.f);
	InitCastingSpeed(0.f);
	InitCriticalRate(0.f);
	InitCriticalDamage(1.f);
	InitEXP(0.f);
	InitLevel(1.f);
}

void UPlayerAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	// CriticalRate 클램핑 (0.0 ~ 1.0)
	if (Attribute == GetCriticalRateAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, 1.f);
		return;
	}

	// 나머지 어트리뷰트 — 음수 방지
	ClampPositive(Attribute, NewValue);
}

void UPlayerAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetEXPAttribute())
	{
		// EXP 하한 보정
		SetEXP(FMath::Max(GetEXP(), 0.f));

		// EXP 변경 이벤트 발행 — 레벨업 시스템이 구독하여 레벨업 판정
		OnEXPChangedDel.Broadcast(GetEXP(), static_cast<int32>(GetLevel()));
	}

	if (Data.EvaluatedData.Attribute == GetLevelAttribute())
	{
		// Level 하한 보정 (최소 1)
		SetLevel(FMath::Max(GetLevel(), 1.f));
	}
}

void UPlayerAttributeSet::ClampPositive(const FGameplayAttribute& Attribute, float& NewValue) const
{
	// DEF, ATK, AttackSpeed, CastingSpeed, CriticalDamage, EXP, Level 모두 0 이상
	if (Attribute == GetATKAttribute()          ||
		Attribute == GetDEFAttribute()          ||
		Attribute == GetAttackSpeedAttribute()  ||
		Attribute == GetCastingSpeedAttribute() ||
		Attribute == GetCriticalDamageAttribute()||
		Attribute == GetEXPAttribute()          ||
		Attribute == GetLevelAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.f);
	}
}