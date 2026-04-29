// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/RSHUDWidget.h"
#include "UI/Ingame/SlotContainerWidget.h"
#include "UI/Ingame/CharacterStatPopupWidget.h"
#include "UI/Enemy/BossHPBarWidget.h"
#include "AbilitySystemComponent.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Animation/WidgetAnimation.h"
#include "System/LoggingSystem.h"

URSHUDWidget::URSHUDWidget()
{
}

void URSHUDWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (Btn_StatPopup)
	{
		Btn_StatPopup->OnClicked.AddDynamic(this, &URSHUDWidget::OnStatPopupBtnClicked);
	}

	// 팝업은 기본 숨김
	if (WBP_CharacterStatPopup)
	{
		WBP_CharacterStatPopup->SetVisibility(ESlateVisibility::Collapsed);
	}

	// BossHPBar는 보스 스폰 전까지 숨김
	if (WBP_BossHPBar)
	{
		WBP_BossHPBar->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void URSHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
	ensureMsgf(WBP_SlotContainer, TEXT("SlotContainerWidget BindWidget 누락"));
}

void URSHUDWidget::ToggleStatPopup()
{
	if (!WBP_CharacterStatPopup)
	{
		return;
	}

	if (WBP_CharacterStatPopup->IsOpen())
	{
		WBP_CharacterStatPopup->CloseUI();
	}
	else
	{
		WBP_CharacterStatPopup->OpenUI();
	}
}

void URSHUDWidget::OnStatPopupBtnClicked()
{
	ToggleStatPopup();
}

void URSHUDWidget::ShowBossHPBar(UAbilitySystemComponent* InASC, float InPhase2Ratio)
{
	if (!ensureMsgf(WBP_BossHPBar, TEXT("RSHUDWidget: WBP_BossHPBar BindWidget 누락. WBP_HUD에 자식 배치 확인 필요.")))
	{
		return;
	}

	WBP_BossHPBar->SetVisibility(ESlateVisibility::Visible);
	WBP_BossHPBar->BindToASC(InASC, InPhase2Ratio);
}

void URSHUDWidget::HideBossHPBar()
{
	if (WBP_BossHPBar)
	{
		WBP_BossHPBar->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void URSHUDWidget::FlashDamageIndicator()
{
	if (!Anim_DamageFlash)
	{
		KHS_WARN(TEXT("Anim_DamageFlash 미할당 — WBP_HUD에 애니메이션 생성 필요"));
		return;
	}

	// 재생 중이면 처음부터 다시 재생 (연속 피격 대응)
	StopAnimation(Anim_DamageFlash);
	PlayAnimation(Anim_DamageFlash);
}
