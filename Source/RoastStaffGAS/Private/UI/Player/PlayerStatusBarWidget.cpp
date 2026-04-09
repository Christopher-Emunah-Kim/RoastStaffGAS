// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Player/PlayerStatusBarWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "AbilitySystemComponent.h"
#include "RoastStaffGAS.h"
#include "GAS/Attributes/BaseAttributeSet.h"
#include "GAS/Attributes/PlayerAttributeSet.h"
#include "Subsystems/GameDataSubsystem.h"
#include "System/LoggingSystem.h"
#include "Character/Player/RSPlayerState.h"

void UPlayerStatusBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Pawn 타이밍 안전 보장을 위해 1프레임 지연 후 자체 바인딩
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UPlayerStatusBarWidget::BindToPlayerASC);
}


void UPlayerStatusBarWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	UpdateGhostBar(InDeltaTime);
	
	UpdateExpBar(InDeltaTime);
}

void UPlayerStatusBarWidget::NativeDestruct()
{
	if (CachedASC)
	{
		CachedASC->GetGameplayAttributeValueChangeDelegate(UBaseAttributeSet::GetCurrentHPAttribute())
			.RemoveAll(this);

		CachedASC->GetGameplayAttributeValueChangeDelegate(UBaseAttributeSet::GetMaxHPAttribute())
			.RemoveAll(this);

		// [CHG] 2026-03-28: EXP/Level 구독 해제
		CachedASC->GetGameplayAttributeValueChangeDelegate(UPlayerAttributeSet::GetEXPAttribute())
			.RemoveAll(this);

		CachedASC->GetGameplayAttributeValueChangeDelegate(UPlayerAttributeSet::GetLevelAttribute())
			.RemoveAll(this);
	}

	Super::NativeDestruct();
}


void UPlayerStatusBarWidget::BindToPlayerASC()
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



void UPlayerStatusBarWidget::BindToASC(UAbilitySystemComponent* InASC)
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

	if (Txt_CurHP)
	{
		Txt_CurHP->SetText(FText::AsNumber(FMath::FloorToInt(TargetHealth)));
	}
	if (Txt_MaxHP)
	{
		Txt_MaxHP->SetText(FText::AsNumber(FMath::FloorToInt(CurrentMaxHealth)));
	}

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
	BindToAttributeChangeDelegates();

	// EXP 바 초기값 즉시 렌더링
	InitializeEXPBar();

	KHS_WARN(TEXT("어트리뷰트 변경 구독 완료"));
}


void UPlayerStatusBarWidget::BindToAttributeChangeDelegates()
{
	// HP 어트리뷰트 구독
	CachedASC->GetGameplayAttributeValueChangeDelegate(UBaseAttributeSet::GetCurrentHPAttribute())
			 .AddUObject(this, &UPlayerStatusBarWidget::OnCurrentHPChanged);

	CachedASC->GetGameplayAttributeValueChangeDelegate(UBaseAttributeSet::GetMaxHPAttribute())
			 .AddUObject(this, &UPlayerStatusBarWidget::OnMaxHPChanged);

	// EXP 어트리뷰트 구독
	CachedASC->GetGameplayAttributeValueChangeDelegate(UPlayerAttributeSet::GetEXPAttribute())
			 .AddUObject(this, &UPlayerStatusBarWidget::OnEXPAttrChanged);

	// Level 어트리뷰트 구독
	CachedASC->GetGameplayAttributeValueChangeDelegate(UPlayerAttributeSet::GetLevelAttribute())
			 .AddUObject(this, &UPlayerStatusBarWidget::OnLevelAttrChanged);
}


void UPlayerStatusBarWidget::InitializeEXPBar()
{
	const float InitEXP   = CachedASC->GetNumericAttribute(UPlayerAttributeSet::GetEXPAttribute());
	const int32 InitLevel = static_cast<int32>(CachedASC->GetNumericAttribute(UPlayerAttributeSet::GetLevelAttribute()));

	GET_GI_SUBSYSTEM_FROM(UGameDataSubsystem, GDS, GetWorld()->GetGameInstance())
	float MaxEXP = 0.f;
	// RequiredEXP 커브는 "레벨 N에 도달하는 데 필요한 EXP" — CheckLevelUp과 동일하게 +1
	if (GDS->GetLevelCurveValue(FName("RequiredEXP"), InitLevel + 1, MaxEXP) && MaxEXP > 0.f)
	{
		CurrentEXPPercent = FMath::Clamp(InitEXP / MaxEXP, 0.f, 1.f);
	}
	else
	{
		CurrentEXPPercent = 1.f; // 최대 레벨 또는 데이터 미정의 → 바 가득 표시
	}
	TargetEXPPercent = CurrentEXPPercent;

	if (PBar_Exp)
	{
		PBar_Exp->SetPercent(CurrentEXPPercent);
	}

	if (Txt_CurExp)
	{
		Txt_CurExp->SetText(FText::AsNumber(FMath::FloorToInt(InitEXP)));
	}
	if (Txt_MaxExp && MaxEXP > 0.f)
	{
		Txt_MaxExp->SetText(FText::AsNumber(FMath::FloorToInt(MaxEXP)));
	}
}

void UPlayerStatusBarWidget::OnCurrentHPChanged(const FOnAttributeChangeData& Data)
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

	if (Txt_CurHP)
	{
		Txt_CurHP->SetText(FText::AsNumber(FMath::FloorToInt(TargetHealth)));
	}

	// GhostBar 보간 대기 리셋
	GhostDelayTimer = GhostDelayTime;

	CheckLowHealthState();
}

void UPlayerStatusBarWidget::OnMaxHPChanged(const FOnAttributeChangeData& Data)
{
	CurrentMaxHealth = Data.NewValue;
	if (Txt_MaxHP)
	{
		Txt_MaxHP->SetText(FText::AsNumber(FMath::FloorToInt(CurrentMaxHealth)));
	}
}

void UPlayerStatusBarWidget::UpdateGhostBar(float InDeltaTime)
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


void UPlayerStatusBarWidget::UpdateExpBar(float InDeltaTime)
{
	if (bIsLerpingEXP && PBar_Exp)
	{
		CurrentEXPPercent = FMath::FInterpTo(CurrentEXPPercent, TargetEXPPercent, InDeltaTime, EXPLerpSpeed);
		PBar_Exp->SetPercent(CurrentEXPPercent);

		if (FMath::IsNearlyEqual(CurrentEXPPercent, TargetEXPPercent, 0.001f))
		{
			CurrentEXPPercent = TargetEXPPercent;
			PBar_Exp->SetPercent(CurrentEXPPercent);
			bIsLerpingEXP = false;
		}
	}
}

void UPlayerStatusBarWidget::CheckLowHealthState()
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

void UPlayerStatusBarWidget::TriggerHitShake()
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

void UPlayerStatusBarWidget::OnLevelAttrChanged(const FOnAttributeChangeData& Data)
{
	// 레벨업 감지 — 현재 EXP 바 퍼센트를 Lerp 시작점으로 캐시
	LerpStartPercent = CurrentEXPPercent;

	if (Txt_MaxExp)
	{
		const int32 NewLevel = FMath::FloorToInt(Data.NewValue);
		GET_GI_SUBSYSTEM_FROM(UGameDataSubsystem, GDS, GetWorld()->GetGameInstance())
		float MaxEXP = 0.f;
		if (GDS->GetLevelCurveValue(FName("RequiredEXP"), NewLevel + 1, MaxEXP) && MaxEXP > 0.f)
		{
			Txt_MaxExp->SetText(FText::AsNumber(FMath::FloorToInt(MaxEXP)));
		}
	}
}

void UPlayerStatusBarWidget::OnEXPAttrChanged(const FOnAttributeChangeData& Data)
{
	// EXP 변화 → 현재 Level 기준 MaxEXP 조회 후 Lerp 목표 설정
	if (!CachedASC)
	{
		return;
	}

	const int32 CurrentLevel = static_cast<int32>(CachedASC->GetNumericAttribute(UPlayerAttributeSet::GetLevelAttribute()));

	GET_GI_SUBSYSTEM_FROM(UGameDataSubsystem, GDS, GetWorld()->GetGameInstance())

	float MaxEXP = 0.f;
	// RequiredEXP 커브는 "레벨 N에 도달하는 데 필요한 EXP" — CheckLevelUp과 동일하게 +1
	if (!GDS->GetLevelCurveValue(FName("RequiredEXP"), CurrentLevel + 1, MaxEXP) || MaxEXP <= 0.f)
	{
		TargetEXPPercent = 1.f; // 최대 레벨 또는 데이터 미정의 → 바 가득
		bIsLerpingEXP    = true;
		return;
	}

	TargetEXPPercent = FMath::Clamp(Data.NewValue / MaxEXP, 0.f, 1.f);
	bIsLerpingEXP    = true;

	if (Txt_CurExp) { Txt_CurExp->SetText(FText::AsNumber(FMath::FloorToInt(Data.NewValue))); }
}

float UPlayerStatusBarWidget::CalcPercent(float InHealth) const
{
	if (CurrentMaxHealth <= 0.f)
	{
		return 0.f;
	}

	return FMath::Clamp(InHealth / CurrentMaxHealth, 0.f, 1.f);
}
