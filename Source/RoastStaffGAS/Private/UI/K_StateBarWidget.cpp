// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/K_StateBarWidget.h"
#include "AbilitySystem/Attributes/K_BaseAttributeSet.h"
#include "System/K_LoggingSystem.h"

#include "AbilitySystemComponent.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UK_StateBarWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UK_StateBarWidget::NativeDestruct()
{
	//델리게이트 구독 해제
	if (BoundASC)
	{
		BoundASC->GetGameplayAttributeValueChangeDelegate(UK_BaseAttributeSet::GetHealthAttribute()).RemoveAll(this);
		BoundASC->GetGameplayAttributeValueChangeDelegate(UK_BaseAttributeSet::GetMaxHealthAttribute()).RemoveAll(this);
		BoundASC->GetGameplayAttributeValueChangeDelegate(UK_BaseAttributeSet::GetManaAttribute()).RemoveAll(this);
		BoundASC->GetGameplayAttributeValueChangeDelegate(UK_BaseAttributeSet::GetMaxManaAttribute()).RemoveAll(this);
	}
	
	Super::NativeDestruct();
}

void UK_StateBarWidget::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	CachedHealth = Data.NewValue;
	SetHealthValues(CachedHealth, CachedMaxHealth);
	
	KHS_SCREEN_INFO(TEXT("[StateBarUI] Health Chacned : %.1f -> %.1f"), Data.OldValue, Data.NewValue);
}

void UK_StateBarWidget::OnMaxHealthChanged(const FOnAttributeChangeData& Data)
{
	CachedMaxHealth = Data.NewValue;
	SetHealthValues(CachedHealth, CachedMaxHealth);
}

void UK_StateBarWidget::OnManaChanged(const FOnAttributeChangeData& Data)
{
	CachedMana = Data.NewValue;
	SetManaValues(CachedMana, CachedMaxMana);
	
	KHS_SCREEN_INFO(TEXT("[StateBarUI] Mana Chacned : %.1f -> %.1f"), Data.OldValue, Data.NewValue);
}

void UK_StateBarWidget::OnMaxManaChanged(const FOnAttributeChangeData& Data)
{
	CachedMaxMana = Data.NewValue;
	SetManaValues(CachedMana, CachedMaxMana);
}

void UK_StateBarWidget::SetHealthValues(float CurrentHealth, float MaxHealth)
{
	if (!bar_Health || !txt_Health)
	{
		KHS_WARN(TEXT("[StateBarUI] bar_Health / txt_Health are invalid"));
	}
	
	const float percent = (MaxHealth > 0.f) ? (CurrentHealth / MaxHealth) : 0.f;
	bar_Health->SetPercent(percent);
	
	const FString txt = FString::Printf(TEXT("%.0f / %.0f"), CurrentHealth, MaxHealth);
	txt_Health->SetText(FText::FromString(txt));
}

void UK_StateBarWidget::SetManaValues(float CurrentMana, float MaxMana)
{
	if (!bar_Mana || !txt_Mana)
	{
		KHS_WARN(TEXT("[StateBarUI] bar_mana / txt_mana are invalid"));
	}
	
	const float percent = (MaxMana > 0.f) ? (CurrentMana / MaxMana) : 0.f;
	bar_Mana->SetPercent(percent);
	
	const FString txt = FString::Printf(TEXT("%.0f / %.0f"), CurrentMana, MaxMana);
	txt_Mana->SetText(FText::FromString(txt));
}

void UK_StateBarWidget::BindToASC(UAbilitySystemComponent* InASC)
{
	if (!ensureMsgf(InASC, TEXT("[StateBarUI] BindToASC - Invalid ASC")))
	{
		return;
	}
	
	//기존 바인딩 해제
	if (BoundASC)
	{
		BoundASC->GetGameplayAttributeValueChangeDelegate(UK_BaseAttributeSet::GetHealthAttribute()).RemoveAll(this);
		BoundASC->GetGameplayAttributeValueChangeDelegate(UK_BaseAttributeSet::GetMaxHealthAttribute()).RemoveAll(this);
		BoundASC->GetGameplayAttributeValueChangeDelegate(UK_BaseAttributeSet::GetManaAttribute()).RemoveAll(this);
		BoundASC->GetGameplayAttributeValueChangeDelegate(UK_BaseAttributeSet::GetMaxManaAttribute()).RemoveAll(this);
	}
	
	BoundASC = InASC;
	
	//State 변경 감지 구독
	BoundASC->GetGameplayAttributeValueChangeDelegate(UK_BaseAttributeSet::GetHealthAttribute()).AddUObject(this, &UK_StateBarWidget::OnHealthChanged);
	BoundASC->GetGameplayAttributeValueChangeDelegate(UK_BaseAttributeSet::GetMaxHealthAttribute()).AddUObject(this, &UK_StateBarWidget::OnMaxHealthChanged);
	BoundASC->GetGameplayAttributeValueChangeDelegate(UK_BaseAttributeSet::GetManaAttribute()).AddUObject(this, &UK_StateBarWidget::OnManaChanged);
	BoundASC->GetGameplayAttributeValueChangeDelegate(UK_BaseAttributeSet::GetMaxManaAttribute()).AddUObject(this, &UK_StateBarWidget::OnMaxManaChanged);
	
	//UI업데이트
	UpdateStateBarDisplay();
	
	KHS_INFO(TEXT("[StateBarUI] Bound to ASC Successfully"));
}

void UK_StateBarWidget::UpdateStateBarDisplay()
{
	if (!ensureMsgf(BoundASC, TEXT("Invalid BoundASC")))
	{
		return;
	}
	
	//ASC에서 현재 Attribute 값 로드
	bool bFound = false;
	
	CachedHealth = BoundASC->GetGameplayAttributeValue(UK_BaseAttributeSet::GetHealthAttribute(), bFound);
	CachedMaxHealth = BoundASC->GetGameplayAttributeValue(UK_BaseAttributeSet::GetMaxHealthAttribute(), bFound);
	CachedMana = BoundASC->GetGameplayAttributeValue(UK_BaseAttributeSet::GetManaAttribute(), bFound);
	CachedMaxMana = BoundASC->GetGameplayAttributeValue(UK_BaseAttributeSet::GetMaxManaAttribute(), bFound);
	
	SetHealthValues(CachedHealth, CachedMaxHealth);
	SetManaValues(CachedMana, CachedMaxMana);
}
