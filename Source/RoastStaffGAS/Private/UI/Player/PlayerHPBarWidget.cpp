// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Player/PlayerHPBarWidget.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"
#include "AbilitySystemComponent.h"
#include "GAS/Attributes/BaseAttributeSet.h"
#include "System/LoggingSystem.h"
#include "Character/Player/RSPlayerState.h"

void UPlayerHPBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Pawn 타이밍 안전 보장을 위해 1프레임 지연 후 자체 바인딩
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UPlayerHPBarWidget::BindToPlayerASC);
}

void UPlayerHPBarWidget::BindToPlayerASC()
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		KHS_WARN(TEXT("PlayerHPBarWidget::BindToPlayerASC: OwningPlayer is null."));
		return;
	}

	ARSPlayerState* PS = PC->GetPlayerState<ARSPlayerState>();
	if (!PS)
	{
		KHS_WARN(TEXT("PlayerHPBarWidget::BindToPlayerASC: PlayerState is null."));
		return;
	}

	UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
	if (!ASC)
	{
		KHS_WARN(TEXT("PlayerHPBarWidget::BindToPlayerASC: ASC is null."));
		return;
	}

	KHS_WARN(TEXT("BindToPlayerASC 진입"));
	BindToASC(ASC);
}

void UPlayerHPBarWidget::BindToASC(UAbilitySystemComponent* InASC)
{
	if (!InASC)
	{
		KHS_WARN(TEXT("PlayerHPBarWidget::BindToASC: InASC is null."));
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
		.AddUObject(this, &UPlayerHPBarWidget::OnCurrentHPChanged);

	CachedASC->GetGameplayAttributeValueChangeDelegate(UBaseAttributeSet::GetMaxHPAttribute())
		.AddUObject(this, &UPlayerHPBarWidget::OnMaxHPChanged);
	
	KHS_WARN(TEXT("어트리뷰트 변경 구독 완료"));
}

void UPlayerHPBarWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	UpdateGhostBar(InDeltaTime);
}

void UPlayerHPBarWidget::NativeDestruct()
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

void UPlayerHPBarWidget::OnCurrentHPChanged(const FOnAttributeChangeData& Data)
{
	if (FMath::IsNearlyEqual(TargetHealth, Data.NewValue, 0.01f))
	{
		return;
	}

	// HP 감소 시 HitShake 재생 (증가/회복은 제외)
	if (Data.NewValue < Data.OldValue)
	{
		TriggerHitShake();
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

void UPlayerHPBarWidget::OnMaxHPChanged(const FOnAttributeChangeData& Data)
{
	CurrentMaxHealth = Data.NewValue;
}

void UPlayerHPBarWidget::UpdateGhostBar(float InDeltaTime)
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

void UPlayerHPBarWidget::CheckLowHealthState()
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

void UPlayerHPBarWidget::TriggerHitShake()
{
	if (!Anim_HitShake)
	{
		return;
	}

	// 이미 재생 중이면 처음부터 재시작
	if (IsAnimationPlaying(Anim_HitShake))
	{
		StopAnimation(Anim_HitShake);
	}

	PlayAnimation(Anim_HitShake);
}

float UPlayerHPBarWidget::CalcPercent(float InHealth) const
{
	if (CurrentMaxHealth <= 0.f)
	{
		return 0.f;
	}

	return FMath::Clamp(InHealth / CurrentMaxHealth, 0.f, 1.f);
}
