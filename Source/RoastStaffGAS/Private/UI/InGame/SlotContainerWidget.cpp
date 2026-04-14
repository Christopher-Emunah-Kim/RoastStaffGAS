// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Ingame/SlotContainerWidget.h"
#include "UI/Ingame/CharacterSkillSlotWidget.h"
#include "UI/Ingame/WeaponSlotWidget.h"
#include "System/LoggingSystem.h"

void USlotContainerWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ensureMsgf(SkillSlotWidget_0, TEXT("SkillSlotWidget_0 BindWidget 누락"));
	ensureMsgf(SkillSlotWidget_1, TEXT("SkillSlotWidget_1 BindWidget 누락"));
	ensureMsgf(SlotWidget_0,      TEXT("SlotWidget_0 BindWidget 누락"));
	ensureMsgf(SlotWidget_1,      TEXT("SlotWidget_1 BindWidget 누락"));
	ensureMsgf(SlotWidget_2,      TEXT("SlotWidget_2 BindWidget 누락"));

	SkillSlotWidget_0->InitSlot(0);
	SkillSlotWidget_0->UpdateSlot(nullptr);
	SkillSlotWidget_1->InitSlot(1);
	SkillSlotWidget_1->UpdateSlot(nullptr);
	SlotWidget_0->InitSlot(0);
	SlotWidget_0->UpdateSlot(nullptr);
	SlotWidget_1->InitSlot(1);
	SlotWidget_1->UpdateSlot(nullptr);
	SlotWidget_2->InitSlot(2);
	SlotWidget_2->UpdateSlot(nullptr);
}

UWeaponSlotWidget* USlotContainerWidget::GetWeaponSlotWidget(int32 SlotIndex) const
{
	switch (SlotIndex)
	{
	case 0: return SlotWidget_0;
	case 1: return SlotWidget_1;
	case 2: return SlotWidget_2;
	default:
		KHS_WARN(TEXT("[SlotContainer] 유효하지 않은 WeaponSlotIndex: %d"), SlotIndex);
		return nullptr;
	}
}

UCharacterSkillSlotWidget* USlotContainerWidget::GetSkillSlotWidget(int32 SlotIndex) const
{
	switch (SlotIndex)
	{
	case 0: return SkillSlotWidget_0;
	case 1: return SkillSlotWidget_1;
	default:
		KHS_WARN(TEXT("[SlotContainer] 유효하지 않은 SkillSlotIndex: %d"), SlotIndex);
		return nullptr;
	}
}
