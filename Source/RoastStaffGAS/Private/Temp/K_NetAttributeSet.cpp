// Fill out your copyright notice in the Description page of Project Settings.


#include "Temp/K_NetAttributeSet.h"

#include "Net/UnrealNetwork.h"

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
}
