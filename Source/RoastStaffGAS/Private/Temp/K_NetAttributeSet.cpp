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
		KHS_INFO(TEXT("[%s] ItemCount changed in PostGE: %.1f"), 
			GetWorld()->GetNetMode() == NM_ListenServer ? TEXT("SERVER") : TEXT("CLIENT"),
			newValue);
		
		UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
		KHS_INFO(TEXT("ASC is %s"), ASC ? TEXT("VALID") : TEXT("NULL")); 
	
		if (!ensureMsgf(ASC, TEXT("fail to get owning ASC")))
		{
			return;
		}
	
		FOnAttributeChangeData changedData;
		changedData.Attribute = GetItemCountAttribute();
		changedData.OldValue = newValue-Data.EvaluatedData.Magnitude;
		changedData.NewValue = newValue;
	
		KHS_INFO(TEXT("[%s] Manually broadcasted ItemCount change in PostGE"), 
			   GetWorld()->GetNetMode() == NM_ListenServer ? TEXT("SERVER") : TEXT("CLIENT"));
	
		ASC->GetGameplayAttributeValueChangeDelegate(GetItemCountAttribute()).Broadcast(changedData);
		KHS_INFO(TEXT("Broadcast completed!"));
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
	KHS_INFO(TEXT("OnRep_ItemCount called!"));
	
	GAMEPLAYATTRIBUTE_REPNOTIFY(UK_NetAttributeSet, ItemCount, OldValue);
	
	// UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
	// KHS_INFO(TEXT("ASC is %s"), ASC ? TEXT("VALID") : TEXT("NULL")); 
	//
	// if (!ensureMsgf(ASC, TEXT("fail to get owning ASC")))
	// {
	// 	return;
	// }
	//
	// FOnAttributeChangeData changedData;
	// changedData.Attribute = GetItemCountAttribute();
	// changedData.OldValue = OldValue.GetCurrentValue();
	// changedData.NewValue = GetItemCount();
	//
	// KHS_INFO(TEXT("Broadcasting manual delegate: %.1f -> %.1f"), 
	// 		changedData.OldValue, changedData.NewValue); 
	//
	// ASC->GetGameplayAttributeValueChangeDelegate(GetItemCountAttribute()).Broadcast(changedData);
	// KHS_INFO(TEXT("Broadcast completed!"));
	
	
	//리플리케이션 확인용 로그
	const float oldVal = OldValue.GetCurrentValue();
	const float newVal = GetItemCount();
	KHS_SCREEN_INFO(TEXT("[%s] ItemCount replicated: %.1f -> %.1f"), 
		GetWorld()->GetNetMode() == NM_DedicatedServer ? TEXT("SERVER") : TEXT("CLIENT"),
		oldVal, newVal);
}
