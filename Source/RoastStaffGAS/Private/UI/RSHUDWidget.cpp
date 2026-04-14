// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/RSHUDWidget.h"
#include "UI/Ingame/SlotContainerWidget.h"
#include "UI/Ingame/CharacterStatPopupWidget.h"
#include "Components/Button.h"

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
}

void URSHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
	ensureMsgf(WBP_SlotContainer, TEXT("SlotContainerWidget BindWidget 누락"));
}

void URSHUDWidget::ToggleStatPopup()
{
	if (!WBP_CharacterStatPopup) return;

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
