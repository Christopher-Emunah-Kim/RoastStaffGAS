// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WeaponSlotWidget.h"
  #include "Data/RuntimeDataStructs.h"
  #include "Components/TextBlock.h"
  #include "Components/Image.h"
  #include "System/LoggingSystem.h"



UWeaponSlotWidget::UWeaponSlotWidget()
{
	
}

void UWeaponSlotWidget::InitSlot(int32 InSlotIndex)
{
	SlotIndex = InSlotIndex;

}

void UWeaponSlotWidget::UpdateSlot(const FWeaponSlotInstanceData* SlotData)
{
	if (!SlotData || SlotData->IsEmpty()) //무기 장착이 일어나지않은 슬롯
	{
		Txt_WeaponName->SetText(FText::FromString(TEXT("EMPTY")));
		Img_CooldownOverlay->SetVisibility(ESlateVisibility::Collapsed);
		Txt_CooldownRemaining->SetVisibility(ESlateVisibility::Collapsed);
		Img_ActiveBorder->SetVisibility(ESlateVisibility::Collapsed);
		bIsCooldownActive = false;
		return;
	}

	Txt_WeaponName->SetText(FText::FromName(SlotData->EquipData.WeaponName));
	Img_ActiveBorder->SetVisibility(SlotData->bIsActive ? ESlateVisibility::Visible : ESlateVisibility::Collapsed); // 액티브 모드 여부

	// 쿨타임
	if (SlotData->CooldownRemaining > 0.f)
	{
		LocalCooldownRemaining = SlotData->CooldownRemaining;
		bIsCooldownActive = true;
		Img_CooldownOverlay->SetVisibility(ESlateVisibility::Visible);
		Txt_CooldownRemaining->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		bIsCooldownActive = false;
		Img_CooldownOverlay->SetVisibility(ESlateVisibility::Collapsed);
		Txt_CooldownRemaining->SetVisibility(ESlateVisibility::Collapsed);
	}

}

void UWeaponSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	ensureMsgf(Txt_WeaponName,        TEXT("Txt_WeaponName BindWidget 누락"));
	ensureMsgf(Img_CooldownOverlay,    TEXT("Img_CooldownOverlay BindWidget 누락"));
	ensureMsgf(Txt_CooldownRemaining, TEXT("Txt_CooldownRemaining BindWidget 누락"));
	ensureMsgf(Img_ActiveBorder,       TEXT("Img_ActiveBorder BindWidget 누락"));

}

void UWeaponSlotWidget::UpdateCooldown(float InDeltaTime)
{
	if (!bIsCooldownActive)
	{
		return;
	}

	LocalCooldownRemaining -= InDeltaTime;

	if (LocalCooldownRemaining <= 0.f)
	{
		LocalCooldownRemaining = 0.f;
		bIsCooldownActive = false;
		Img_CooldownOverlay->SetVisibility(ESlateVisibility::Collapsed);
		Txt_CooldownRemaining->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	// 1초 미만이면 "0" 표시 
	int32 DisplaySeconds = (LocalCooldownRemaining < 1.0f) ? 0 : FMath::CeilToInt(LocalCooldownRemaining);
	Txt_CooldownRemaining->SetText(FText::AsNumber(DisplaySeconds));
}

void UWeaponSlotWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	UpdateCooldown(InDeltaTime);

}
