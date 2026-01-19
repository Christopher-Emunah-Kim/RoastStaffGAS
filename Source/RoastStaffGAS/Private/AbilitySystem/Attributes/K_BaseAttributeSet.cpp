// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Attributes/K_BaseAttributeSet.h"
#include "AbilitySystem/GameplayTags/K_GameplayTags.h"
#include "Net/UnrealNetwork.h"
#include "System/K_LoggingSystem.h"

UK_BaseAttributeSet::UK_BaseAttributeSet()
{
	//기본값 세팅
	//TODO 데이터 테이블에서 로드하는 방식으로 추후 변경
	InitHealth(100.f);
	InitMaxHealth(100.f);
	
	InitMana(100.f);
	InitMaxMana(100.f);
	
	InitIncomingDamage(0.f);
}

void UK_BaseAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	
	//CurrentValue 클램핑
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
	else if (Attribute == GetMaxHealthAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.f);
	}
	else if (Attribute == GetManaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxMana());
	}
	else if (Attribute == GetMaxManaAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.f);
	}
}

void UK_BaseAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	
	//IncomingDamage 처리
	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		const float damageReceived = GetIncomingDamage();
		SetIncomingDamage(0.f); //메타데이터 리셋
		if (damageReceived > 0.f)
		{
			//Health 감소
			const float newHealth = FMath::Clamp(GetHealth() - damageReceived, 0.f, GetMaxHealth());
			SetHealth(newHealth);
			
			if (newHealth <= 0.f)
			{
				HandleDeath(Data);
			}
			else
			{
				BroadcastDamageEvent(Data, damageReceived);
			}
		}
	}
	
	//BaseValue 클램핑
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
	}
	else if (Data.EvaluatedData.Attribute == GetManaAttribute())
	{
		SetMana(FMath::Clamp(GetMana(), 0.f, GetMaxMana()));
	}
}

void UK_BaseAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	//RPC 등록
	DOREPLIFETIME_CONDITION_NOTIFY(UK_BaseAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UK_BaseAttributeSet, MaxHealth, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UK_BaseAttributeSet, Mana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UK_BaseAttributeSet, MaxMana, COND_None, REPNOTIFY_OnChanged);
}

void UK_BaseAttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UK_BaseAttributeSet, Health, OldValue);
}

void UK_BaseAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UK_BaseAttributeSet, MaxHealth, OldValue);
}

void UK_BaseAttributeSet::OnRep_Mana(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UK_BaseAttributeSet, Mana, OldValue);
}

void UK_BaseAttributeSet::OnRep_MaxMana(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UK_BaseAttributeSet, MaxMana, OldValue);
}

void UK_BaseAttributeSet::HandleDeath(const FGameplayEffectModCallbackData& Data)
{
	//사망 이벤트 전송
	AActor* targetActor = Data.Target.GetAvatarActor();
	if (!ensureMsgf(targetActor, TEXT("targetActor is not valid")))
	{
		return;
	}
	
	FGameplayEventData eventData;
	eventData.Instigator = Data.EffectSpec.GetEffectContext().GetInstigator();
	eventData.Target = targetActor; 
	
	//타겟 액터에 이벤트에 따른 태그 전달
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(targetActor, KTags::Event_Combat_Death, eventData);
	
	KHS_WARN(TEXT("[%s] Death event sent. Instigator : %s"), *targetActor->GetName(), eventData.Instigator? *eventData.Instigator.GetName() : TEXT("NONE"));
}

void UK_BaseAttributeSet::BroadcastDamageEvent(const FGameplayEffectModCallbackData& Data, float Damage)
{
	//데미지 수신 이벤트 전송
	AActor* targetActor = Data.Target.GetAvatarActor();
	if (!ensureMsgf(targetActor, TEXT("targetActor is not valid")))
	{
		return;
	}
	
	FGameplayEventData eventData;
	eventData.Instigator = Data.EffectSpec.GetEffectContext().GetInstigator();
	eventData.Target = targetActor; 
	eventData.EventMagnitude = Damage;
	
	//타겟 액터에 이벤트에 따른 태그 전달
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(targetActor, KTags::Event_Combat_TakeDamage, eventData);
}
