// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/InGame/CharacterSkillSlotWidget.h"

#include "RoastStaffGAS.h"
#include "Subsystems/SkillManagerSubsystem.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "System/LoggingSystem.h"
#include "Subsystems/UIManagerSubsystem.h"

const FName UCharacterSkillSlotWidget::CooldownPercentParam = FName("Percent");

void UCharacterSkillSlotWidget::InitSlot(int32 InSlotIndex)
{
	SlotIndex = InSlotIndex;
}

void UCharacterSkillSlotWidget::UpdateSlot(const FSkillSlotState* SlotState)
{
	// 빈 슬롯 — 스킬 미할당 시 숨김
	if (!SlotState || SlotState->ExecData.SkillID.IsNone())
	{
		SetVisibility(ESlateVisibility::Collapsed);
		bIsCooldownActive = false;
		return;
	}

	SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	// 스킬 이름
	if (Txt_SkillName)
	{
		Txt_SkillName->SetText(SlotState->ExecData.DisplayName);
	}

	// 스킬 아이콘
	if (!SlotState->ExecData.SkillIconSoftRef.IsNull())
	{
		LoadedSkillIcon = SlotState->ExecData.SkillIconSoftRef.LoadSynchronous();
		if (LoadedSkillIcon)
		{
			Img_SkillIcon->SetBrushFromTexture(LoadedSkillIcon);
			Img_SkillIcon->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
	}

	// 쿨타임
	if (SlotState->bIsOnCooldown && SlotState->CooldownRemaining > 0.f)
	{
		TotalCooldown          = SlotState->TotalCooldown;
		LocalCooldownRemaining = SlotState->CooldownRemaining;
		bIsCooldownActive      = true;
		Img_CooldownOverlay->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		Txt_CooldownRemaining->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
	else
	{
		bIsCooldownActive = false;
		Img_CooldownOverlay->SetVisibility(ESlateVisibility::Collapsed);
		Txt_CooldownRemaining->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UCharacterSkillSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ensureMsgf(Img_SkillIcon, TEXT("Img_SkillIcon BindWidget 누락 — SlotIndex %d"), SlotIndex);
	ensureMsgf(Img_CooldownOverlay, TEXT("Img_CooldownOverlay BindWidget 누락 — SlotIndex %d"), SlotIndex);
	ensureMsgf(Txt_CooldownRemaining, TEXT("Txt_CooldownRemaining BindWidget 누락 — SlotIndex %d"), SlotIndex);
	ensureMsgf(Txt_SkillName, TEXT("Txt_SkillName BindWidget 누락 — SlotIndex %d"), SlotIndex);

	if (Img_CooldownOverlay)
	{
		CooldownMID = Img_CooldownOverlay->GetDynamicMaterial();
	}
}

void UCharacterSkillSlotWidget::UpdateCooldown(float InDeltaTime)
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

void UCharacterSkillSlotWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	GET_GI_SUBSYSTEM_FROM(UUIManagerSubsystem, UMS, GetWorld()->GetGameInstance())
	if (UMS->IsAnyPausingUIOpen())
	{
		return;
	}

	UpdateCooldown(InDeltaTime);
}
