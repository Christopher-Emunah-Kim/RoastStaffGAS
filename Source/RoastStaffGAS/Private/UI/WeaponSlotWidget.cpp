// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WeaponSlotWidget.h"
#include "Data/RuntimeDataStructs.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "System/LoggingSystem.h"

const FName UWeaponSlotWidget::CooldownPercentParam = FName("Percent");

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
		Img_SkillIcon->SetVisibility(ESlateVisibility::Collapsed);
		Img_ActiveBorder->SetVisibility(ESlateVisibility::Collapsed);
		bIsCooldownActive = false;
		return;
	}

	FName DisplayName = SlotData->EquipData.WeaponName.IsNone()? SlotData->EquipData.WeaponID : SlotData->EquipData.WeaponName;
	Txt_WeaponName->SetText(FText::FromName(DisplayName));

	Img_ActiveBorder->SetVisibility(SlotData->bIsActive ? ESlateVisibility::Visible : ESlateVisibility::Collapsed); // 액티브 모드 여부
	
	//스킬 아이콘 세팅
	if (!SlotData->EquipData.SkillIcon.IsNull())
	{
		LoadedSkillIcon = SlotData->EquipData.SkillIcon.LoadSynchronous();
		if (LoadedSkillIcon)
		{
			Img_SkillIcon->SetBrushFromTexture(LoadedSkillIcon);
		}
	}
	
	// 쿨타임
	if (SlotData->bIsActive || SlotData->CooldownRemaining <= 0.f)
	{
		bIsCooldownActive = false;
		Img_CooldownOverlay->SetVisibility(ESlateVisibility::Collapsed);
		Txt_CooldownRemaining->SetVisibility(ESlateVisibility::Collapsed);
	}
	else
	{
		TotalCooldown = SlotData->EquipData.Cooldown;
		LocalCooldownRemaining = SlotData->CooldownRemaining;
		bIsCooldownActive = true;
		Img_CooldownOverlay->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		Txt_CooldownRemaining->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

}

void UWeaponSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	ensureMsgf(Txt_WeaponName, TEXT("Txt_WeaponName BindWidget 누락"));
	ensureMsgf(Img_CooldownOverlay, TEXT("Img_CooldownOverlay BindWidget 누락"));
	ensureMsgf(Img_SkillIcon, TEXT("Img_SkillIcon BindWidget 누락"));
	ensureMsgf(Txt_CooldownRemaining, TEXT("Txt_CooldownRemaining BindWidget 누락"));
	ensureMsgf(Img_ActiveBorder, TEXT("Img_ActiveBorder BindWidget 누락"));

	// MID 생성 — BP에서 Img_CooldownOverlay에 머티리얼 할당 후 동작
	if (Img_CooldownOverlay)
	{
		CooldownMID = Img_CooldownOverlay->GetDynamicMaterial();
	}

}

void UWeaponSlotWidget::UpdateCooldown(float InDeltaTime)
{
	if (!bIsCooldownActive)
	{
		return;
	}

	LocalCooldownRemaining -= InDeltaTime;
	if (CooldownMID && TotalCooldown > 0.f)
	{
		float Percent = FMath::Clamp(LocalCooldownRemaining / TotalCooldown, 0.0f, 1.0f);
		CooldownMID->SetScalarParameterValue(CooldownPercentParam, Percent);
	}
	
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
