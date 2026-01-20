// Fill out your copyright notice in the Description page of Project Settings.


#include "Temp/K_NetAttributeSet.h"
#include "RoastStaffGAS.h"

UK_NetAttributeSet::UK_NetAttributeSet()
{
	InitItemCount(0.f);
}

void UK_NetAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	
	if (Attribute == GetItemCountAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, ITEM_MAX_COUNT);
	}
}

void UK_NetAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	
	//ItemCount 변경값 확인
	if (Data.EvaluatedData.Attribute == GetItemCountAttribute())
	{
		const float newValue = GetItemCount();
		KHS_SCREEN_INFO(TEXT("ItemCount changed to: %.1f (Server)"), newValue);
	}

	
	//이후 UI 구현시 필요하지않을까?
}

void UK_NetAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION_NOTIFY(UK_NetAttributeSet, ItemCount, COND_None, REPNOTIFY_Always);
}

void UK_NetAttributeSet::OnRep_ItemCount(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UK_NetAttributeSet, ItemCount, OldValue);
	
	//리플리케이션 확인용 로그
	const float oldVal = OldValue.GetCurrentValue();
	const float newVal = GetItemCount();
	KHS_INFO(TEXT("ItemCount replicated: %.1f -> %.1f (Client)"), oldVal, newVal);
}
