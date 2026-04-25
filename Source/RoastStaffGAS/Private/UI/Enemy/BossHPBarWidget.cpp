// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Enemy/BossHPBarWidget.h"
#include "Components/ProgressBar.h"
#include "AbilitySystemComponent.h"
#include "GAS/Attributes/BaseAttributeSet.h"
#include "System/LoggingSystem.h"
#include "RoastStaffGAS.h"

UBossHPBarWidget::UBossHPBarWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bIsModal = false;
}

void UBossHPBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ensureMsgf(PBar_BossHP, TEXT(" PBar_BossHP 바인딩 누락. WBP 이름 확인 필요."));

	// FadeOut 완료 시 자체 정리 콜백 등록
	if (Anim_FadeOut)
	{
		FWidgetAnimationDynamicEvent FadeOutFinishedEvent;
		FadeOutFinishedEvent.BindDynamic(this, &UBossHPBarWidget::OnFadeOutFinished);
		BindToAnimationFinished(Anim_FadeOut, FadeOutFinishedEvent);
	}
}

void UBossHPBarWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	UpdateGhostBar(InDeltaTime);
}

void UBossHPBarWidget::NativeDestruct()
{
	if (CachedASC)
	{
		CachedASC->GetGameplayAttributeValueChangeDelegate(UBaseAttributeSet::GetCurrentHPAttribute()).RemoveAll(this);
		CachedASC->GetGameplayAttributeValueChangeDelegate(UBaseAttributeSet::GetMaxHPAttribute()).RemoveAll(this);
	}

	Super::NativeDestruct();
}

void UBossHPBarWidget::BindToASC(UAbilitySystemComponent* InASC, float InPhase2Ratio)
{
	if (!ensureMsgf(InASC, TEXT("InASC is null.")))
	{
		return;
	}

	// 풀 재사용 대비 상태 리셋
	bIsClosing       = false;
	bPhase2Triggered = false;
	Phase2Ratio      = FMath::Clamp(InPhase2Ratio, 0.f, 1.f);
	GhostDelayTimer  = 0.f;

	// 기존 ASC 구독 해제 (재바인딩 안전)
	if (CachedASC)
	{
		CachedASC->GetGameplayAttributeValueChangeDelegate(UBaseAttributeSet::GetCurrentHPAttribute()).RemoveAll(this);
		CachedASC->GetGameplayAttributeValueChangeDelegate(UBaseAttributeSet::GetMaxHPAttribute()).RemoveAll(this);
	}

	CachedASC = InASC;

	// 초기값 즉시 조회
	CurrentHP    = CachedASC->GetNumericAttribute(UBaseAttributeSet::GetCurrentHPAttribute());
	CurrentMaxHP = CachedASC->GetNumericAttribute(UBaseAttributeSet::GetMaxHPAttribute());
	GhostHP      = CurrentHP;

	UpdateProgressBars();

	// 어트리뷰트 변경 델리게이트 구독
	CachedASC->GetGameplayAttributeValueChangeDelegate(UBaseAttributeSet::GetCurrentHPAttribute())
		.AddUObject(this, &UBossHPBarWidget::OnCurrentHPChanged);

	CachedASC->GetGameplayAttributeValueChangeDelegate(UBaseAttributeSet::GetMaxHPAttribute())
		.AddUObject(this, &UBossHPBarWidget::OnMaxHPChanged);
}

// ─────────────────────────────────────────────────────────────────────────────
// ASC 콜백
// ─────────────────────────────────────────────────────────────────────────────

void UBossHPBarWidget::OnCurrentHPChanged(const FOnAttributeChangeData& Data)
{
	if (bIsClosing)
	{
		return;
	}

	CurrentHP = Data.NewValue;

	// MaxHP 최신값 재조회 (MaxHP가 먼저 변경된 경우 대비)
	if (CachedASC)
	{
		CurrentMaxHP = CachedASC->GetNumericAttribute(UBaseAttributeSet::GetMaxHPAttribute());
	}

	UpdateProgressBars();

	// GhostBar 보간 대기 리셋
	GhostDelayTimer = GhostDelayTime;

	// Phase2 전환 체크
	if (!bPhase2Triggered && CurrentMaxHP > 0.f)
	{
		const float Ratio = CurrentHP / CurrentMaxHP;
		if (Ratio <= Phase2Ratio)
		{
			OnPhase2Entered();
		}
	}

	// HP=0 → FadeOut 시작
	if (FMath::IsNearlyZero(CurrentHP))
	{
		TriggerFadeOut();
	}
}

void UBossHPBarWidget::OnMaxHPChanged(const FOnAttributeChangeData& Data)
{
	CurrentMaxHP = Data.NewValue;
}

// ─────────────────────────────────────────────────────────────────────────────
// 내부 갱신 헬퍼
// ─────────────────────────────────────────────────────────────────────────────

void UBossHPBarWidget::UpdateProgressBars()
{
	const float Percent = CalcPercent(CurrentHP);

	if (PBar_BossHP)
	{
		PBar_BossHP->SetPercent(Percent);
	}

	if (PBar_Ghost && FMath::IsNearlyZero(GhostDelayTimer))
	{
		PBar_Ghost->SetPercent(CalcPercent(GhostHP));
	}
}

void UBossHPBarWidget::UpdateGhostBar(float InDeltaTime)
{
	if (!PBar_Ghost)
	{
		return;
	}

	if (GhostDelayTimer > 0.f)
	{
		GhostDelayTimer -= InDeltaTime;
		return;
	}

	if (FMath::IsNearlyEqual(GhostHP, CurrentHP, 0.1f))
	{
		return;
	}

	GhostHP = FMath::FInterpTo(GhostHP, CurrentHP, InDeltaTime, InterpSpeed_Ghost);
	PBar_Ghost->SetPercent(CalcPercent(GhostHP));
}

void UBossHPBarWidget::OnPhase2Entered()
{
	bPhase2Triggered = true;

	if (PBar_BossHP)
	{
		PBar_BossHP->SetFillColorAndOpacity(Phase2BarColor);
	}

	KHS_INFO("BossHPBarWidget — Phase2 진입. 색상 전환.");
}

void UBossHPBarWidget::TriggerFadeOut()
{
	if (bIsClosing)
	{
		return;
	}

	if (Anim_FadeOut)
	{
		// FadeOut 애니메이션 재생 → OnFadeOutFinished에서 Collapsed 처리
		bIsClosing = true;
		PlayAnimation(Anim_FadeOut);
	}
	else
	{
		// 애니메이션 없으면 EnemySpawner::OnBossKilled → HUD::HideBossHPBar 폴백 경로로 숨김
		KHS_INFO("BossHPBarWidget — Anim_FadeOut 없음. EnemySpawner 폴백 경로로 정리.");
	}
}

void UBossHPBarWidget::OnFadeOutFinished()
{
	// FadeOut 완료 → HUD 자식이므로 Visibility만 숨김 (RemoveFromParent 불필요)
	SetVisibility(ESlateVisibility::Collapsed);
}

float UBossHPBarWidget::CalcPercent(float InHP) const
{
	if (CurrentMaxHP <= 0.f)
	{
		return 0.f;
	}

	return FMath::Clamp(InHP / CurrentMaxHP, 0.f, 1.f);
}
