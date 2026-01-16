// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/EnemyStateWidget.h"
#include "GameplayEffectTypes.h"
#include "AbilitySystem/Attributes/K_BaseAttributeSet.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "System/K_LoggingSystem.h"

void UEnemyStateWidget::NativeConstruct()
{
	Super::NativeConstruct();	
}

void UEnemyStateWidget::NativeDestruct()
{
	if (BoundASC)
	{
		BoundASC->GetGameplayAttributeValueChangeDelegate(
			UK_BaseAttributeSet::GetHealthAttribute()).RemoveAll(this);
		BoundASC->GetGameplayAttributeValueChangeDelegate(
			UK_BaseAttributeSet::GetMaxHealthAttribute()).RemoveAll(this);
	}
	
	Super::NativeDestruct();
}

void UEnemyStateWidget::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	CachedHealth = Data.NewValue;
	SetHealthValues(CachedHealth, CachedMaxHealth);
	
	KHS_SCREEN_INFO(TEXT("[UEnemyStateWidget] Health Chacned : %.1f -> %.1f"), Data.OldValue, Data.NewValue);
}

void UEnemyStateWidget::OnMaxHealthChanged(const FOnAttributeChangeData& Data)
{
	CachedMaxHealth = Data.NewValue;
	SetHealthValues(CachedHealth, CachedMaxHealth);
}

void UEnemyStateWidget::SetHealthValues(float CurrentHealth, float MaxHealth)
{
	if (!bar_Health || !txt_Health)
	{
		KHS_WARN(TEXT("[UEnemyStateWidget] bar_Health and txt_Health are invalid"));
	}
	
	const float percent = (MaxHealth > 0.f) ? (CurrentHealth / MaxHealth) : 0.f;
	bar_Health->SetPercent(percent);
	
	const FString txt = FString::Printf(TEXT("%.0f / %.0f"), CurrentHealth, MaxHealth);
	txt_Health->SetText(FText::FromString(txt));
}

void UEnemyStateWidget::BindToASC(UAbilitySystemComponent* InASC)
{
	if (!ensureMsgf(InASC, TEXT("[EnemyBarWidget] BindToASC - Invalid ASC")))
	{
		return;
	}
	
	//기존 바인딩 해제
	if (BoundASC)
	{
		BoundASC->GetGameplayAttributeValueChangeDelegate(UK_BaseAttributeSet::GetHealthAttribute()).RemoveAll(this);
		BoundASC->GetGameplayAttributeValueChangeDelegate(UK_BaseAttributeSet::GetMaxHealthAttribute()).RemoveAll(this);
	}
	
	BoundASC = InASC;
	
	//State 변경 감지 구독
	BoundASC->GetGameplayAttributeValueChangeDelegate(UK_BaseAttributeSet::GetHealthAttribute()).AddUObject(this, &UEnemyStateWidget::OnHealthChanged);
	BoundASC->GetGameplayAttributeValueChangeDelegate(UK_BaseAttributeSet::GetMaxHealthAttribute()).AddUObject(this, &UEnemyStateWidget::OnMaxHealthChanged);
	
	//UI업데이트
	UpdateStateDisplay();
	
	KHS_INFO(TEXT("[StateBarUI] Bound to ASC Successfully"));
}

void UEnemyStateWidget::UpdateStateDisplay()
{
	if (!ensureMsgf(BoundASC, TEXT("Invalid BoundASC")))
	{
		return;
	}
	
	//ASC에서 현재 Attribute 값 로드
	bool bFound = false;
	
	CachedHealth = BoundASC->GetGameplayAttributeValue(UK_BaseAttributeSet::GetHealthAttribute(), bFound);
	CachedMaxHealth = BoundASC->GetGameplayAttributeValue(UK_BaseAttributeSet::GetMaxHealthAttribute(), bFound);
	
	SetHealthValues(CachedHealth, CachedMaxHealth);
}
