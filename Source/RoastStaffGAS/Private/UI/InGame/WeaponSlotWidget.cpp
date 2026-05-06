// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Ingame/WeaponSlotWidget.h"

#include "RoastStaffGAS.h"
#include "Data/RuntimeDataStructs.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "System/LoggingSystem.h"
#include "Subsystems/UIManagerSubsystem.h"

const FName UWeaponSlotWidget::CooldownPercentParam = FName("Percent");

void UWeaponSlotWidget::InitSlot(int32 InSlotIndex)
{
	SlotIndex = InSlotIndex;
}

void UWeaponSlotWidget::UpdateSlot(const FWeaponSlotInstanceData* SlotData)
{
	// 빈 슬롯 — 무기 미할당 시 위젯 전체 숨김
	if (!SlotData || SlotData->IsEmpty())
	{
		SetVisibility(ESlateVisibility::Collapsed);
		bIsCooldownActive = false;
		return;
	}

	SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	FName DisplayName = SlotData->SlotEquipData.WeaponName.IsNone() ? SlotData->SlotEquipData.WeaponID : SlotData->SlotEquipData.WeaponName;
	Txt_WeaponName->SetText(FText::FromName(DisplayName));

	// 스킬 아이콘
	if (!SlotData->SlotEquipData.SkillIcon.IsNull())
	{
		LoadedSkillIcon = SlotData->SlotEquipData.SkillIcon.LoadSynchronous();
		if (LoadedSkillIcon)
		{
			Img_SkillIcon->SetBrushFromTexture(LoadedSkillIcon);
			Img_SkillIcon->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
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
		TotalCooldown          = SlotData->SlotEquipData.Cooldown;
		LocalCooldownRemaining = SlotData->CooldownRemaining;
		bIsCooldownActive      = true;
		Img_CooldownOverlay->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		Txt_CooldownRemaining->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
}

void UWeaponSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ensureMsgf(Txt_WeaponName,        TEXT("Txt_WeaponName BindWidget 누락"));
	ensureMsgf(Img_CooldownOverlay,   TEXT("Img_CooldownOverlay BindWidget 누락"));
	ensureMsgf(Img_SkillIcon,         TEXT("Img_SkillIcon BindWidget 누락"));
	ensureMsgf(Txt_CooldownRemaining, TEXT("Txt_CooldownRemaining BindWidget 누락"));

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
		const float Percent = FMath::Clamp(LocalCooldownRemaining / TotalCooldown, 0.f, 1.f);
		CooldownMID->SetScalarParameterValue(CooldownPercentParam, Percent);
	}

	if (LocalCooldownRemaining <= 0.f)
	{
		LocalCooldownRemaining = 0.f;
		bIsCooldownActive      = false;
		Img_CooldownOverlay->SetVisibility(ESlateVisibility::Collapsed);
		Txt_CooldownRemaining->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	const int32 DisplaySeconds = (LocalCooldownRemaining < 1.f) ? 0 : FMath::CeilToInt(LocalCooldownRemaining);
	Txt_CooldownRemaining->SetText(FText::AsNumber(DisplaySeconds));
}

void UWeaponSlotWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	GET_GI_SUBSYSTEM_FROM(UUIManagerSubsystem, UMS, GetWorld()->GetGameInstance())
	if (UMS->IsAnyPausingUIOpen())
	{
		return;
	}

	UpdateCooldown(InDeltaTime);
}
