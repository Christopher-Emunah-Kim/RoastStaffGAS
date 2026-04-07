// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Enemy/EnemyHPBarWidget.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "AbilitySystemComponent.h"
#include "GAS/Attributes/BaseAttributeSet.h"
#include "System/LoggingSystem.h"

void UEnemyHPBarWidget::SetEnemyName(const FText& InName)
{
	if (Txt_EnemyName)
	{
		Txt_EnemyName->SetText(InName);
	}
}

void UEnemyHPBarWidget::BindToASC(UAbilitySystemComponent* InASC)
{
	if (!InASC)
	{
		KHS_WARN(TEXT("BindToASC: InASC is null. HPBar will not be bound."));
		return;
	}

	CachedASC = InASC;

	// 초기값 즉시 조회 — 위젯 최초 렌더링
	TargetHealth     = CachedASC->GetNumericAttribute(UBaseAttributeSet::GetCurrentHPAttribute());
	CurrentMaxHealth = CachedASC->GetNumericAttribute(UBaseAttributeSet::GetMaxHPAttribute());
	GhostHealth      = TargetHealth;

	if (PBar_Health)
	{
		PBar_Health->SetPercent(CalcPercent(TargetHealth));
	}

	if (PBar_Ghost)
	{
		PBar_Ghost->SetPercent(CalcPercent(GhostHealth));
	}

	// 초기 LowHealth 상태 적용
	CheckLowHealthState();

	// 어트리뷰트 변경 델리게이트 구독
	CachedASC->GetGameplayAttributeValueChangeDelegate(UBaseAttributeSet::GetCurrentHPAttribute())
		.AddUObject(this, &UEnemyHPBarWidget::OnCurrentHPChanged);

	CachedASC->GetGameplayAttributeValueChangeDelegate(UBaseAttributeSet::GetMaxHPAttribute())
		.AddUObject(this, &UEnemyHPBarWidget::OnMaxHPChanged);
}

void UEnemyHPBarWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	UpdateGhostBar(InDeltaTime);
}

void UEnemyHPBarWidget::NativeDestruct()
{
	if (CachedASC)
	{
		CachedASC->GetGameplayAttributeValueChangeDelegate(UBaseAttributeSet::GetCurrentHPAttribute())
			.RemoveAll(this);

		CachedASC->GetGameplayAttributeValueChangeDelegate(UBaseAttributeSet::GetMaxHPAttribute())
			.RemoveAll(this);
	}

	Super::NativeDestruct();
}

void UEnemyHPBarWidget::OnCurrentHPChanged(const FOnAttributeChangeData& Data)
{
	if (FMath::IsNearlyEqual(TargetHealth, Data.NewValue, 0.01f))
	{
		return;
	}

	TargetHealth = Data.NewValue;

	// MaxHP 최신값 재조회 (MaxHP가 먼저 변경된 경우 대비)
	if (CachedASC)
	{
		CurrentMaxHealth = CachedASC->GetNumericAttribute(UBaseAttributeSet::GetMaxHPAttribute());
	}

	if (PBar_Health)
	{
		PBar_Health->SetPercent(CalcPercent(TargetHealth));
	}

	// GhostBar 보간 대기 리셋
	GhostDelayTimer = GhostDelayTime;

	CheckLowHealthState();
}

void UEnemyHPBarWidget::OnMaxHPChanged(const FOnAttributeChangeData& Data)
{
	CurrentMaxHealth = Data.NewValue;
}

void UEnemyHPBarWidget::UpdateGhostBar(float InDeltaTime)
{
	if (GhostDelayTimer > 0.f)
	{
		GhostDelayTimer -= InDeltaTime;
		return;
	}

	if (!PBar_Ghost)
	{
		return;
	}

	if (FMath::IsNearlyEqual(GhostHealth, TargetHealth, 0.1f))
	{
		return;
	}

	GhostHealth = FMath::FInterpTo(GhostHealth, TargetHealth, InDeltaTime, InterpSpeed_Ghost);
	PBar_Ghost->SetPercent(CalcPercent(GhostHealth));
}

void UEnemyHPBarWidget::CheckLowHealthState()
{
	if (CurrentMaxHealth <= 0.f)
	{
		return;
	}

	const float HealthRatio = TargetHealth / CurrentMaxHealth;
	const bool bShouldBeLowHealth = (HealthRatio <= LowHealthThreshold) && (TargetHealth > 0.f);

	if (bShouldBeLowHealth == bIsLowHealth)
	{
		return;
	}

	bIsLowHealth = bShouldBeLowHealth;

	if (bIsLowHealth)
	{
		if (Img_DangerGlow)
		{
			Img_DangerGlow->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}

		if (Anim_LowHealth)
		{
			PlayAnimation(Anim_LowHealth, 0.f, 0); // 0회 = 무한 반복
		}
	}
	else
	{
		if (Img_DangerGlow)
		{
			Img_DangerGlow->SetVisibility(ESlateVisibility::Hidden);
		}

		if (Anim_LowHealth)
		{
			StopAnimation(Anim_LowHealth);
		}
	}
}

float UEnemyHPBarWidget::CalcPercent(float InHealth) const
{
	if (CurrentMaxHealth <= 0.f)
	{
		return 0.f;
	}

	return FMath::Clamp(InHealth / CurrentMaxHealth, 0.f, 1.f);
}
