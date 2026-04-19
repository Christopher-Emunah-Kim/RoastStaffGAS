// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/InGame/SlotContainerWidget.h"
#include "UI/InGame/CharacterSkillSlotWidget.h"
#include "UI/InGame/WeaponSlotWidget.h"
#include "UI/InGame/PassiveSlotWidget.h"
#include "System/LoggingSystem.h"

void USlotContainerWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ensureMsgf(SkillSlotWidget_0,    TEXT("SkillSlotWidget_0 BindWidget 누락"));
	ensureMsgf(SkillSlotWidget_1,    TEXT("SkillSlotWidget_1 BindWidget 누락"));
	ensureMsgf(SkillSlotWidget_2,    TEXT("SkillSlotWidget_2 BindWidget 누락"));
	ensureMsgf(SkillSlotWidget_3,    TEXT("SkillSlotWidget_3 BindWidget 누락"));
	ensureMsgf(SkillSlotWidget_4,    TEXT("SkillSlotWidget_4 BindWidget 누락"));
	ensureMsgf(SkillSlotWidget_5,    TEXT("SkillSlotWidget_5 BindWidget 누락"));
	ensureMsgf(SlotWidget_0,         TEXT("SlotWidget_0 BindWidget 누락"));
	ensureMsgf(SlotWidget_1,         TEXT("SlotWidget_1 BindWidget 누락"));
	ensureMsgf(SlotWidget_2,         TEXT("SlotWidget_2 BindWidget 누락"));
	ensureMsgf(PassiveSlotWidget_0,  TEXT("PassiveSlotWidget_0 BindWidget 누락"));
	ensureMsgf(PassiveSlotWidget_1,  TEXT("PassiveSlotWidget_1 BindWidget 누락"));
	ensureMsgf(PassiveSlotWidget_2,  TEXT("PassiveSlotWidget_2 BindWidget 누락"));
	ensureMsgf(PassiveSlotWidget_3,  TEXT("PassiveSlotWidget_3 BindWidget 누락"));
	ensureMsgf(PassiveSlotWidget_4,  TEXT("PassiveSlotWidget_4 BindWidget 누락"));
	ensureMsgf(PassiveSlotWidget_5,  TEXT("PassiveSlotWidget_5 BindWidget 누락"));
	ensureMsgf(PassiveSlotWidget_6,  TEXT("PassiveSlotWidget_6 BindWidget 누락"));
	ensureMsgf(PassiveSlotWidget_7,  TEXT("PassiveSlotWidget_7 BindWidget 누락"));

	// 스킬 슬롯 0~5 초기화
	TObjectPtr<UCharacterSkillSlotWidget> SkillSlots[] = {
		SkillSlotWidget_0, SkillSlotWidget_1, SkillSlotWidget_2,
		SkillSlotWidget_3, SkillSlotWidget_4, SkillSlotWidget_5
	};
	for (int32 i = 0; i < static_cast<int32>(UE_ARRAY_COUNT(SkillSlots)); ++i)
	{
		if (SkillSlots[i])
		{
			SkillSlots[i]->InitSlot(i);
			SkillSlots[i]->UpdateSlot(nullptr);
		}
	}
	if (SlotWidget_0)
	{
		SlotWidget_0->InitSlot(0);
		SlotWidget_0->UpdateSlot(nullptr);
	}
	if (SlotWidget_1)
	{
		SlotWidget_1->InitSlot(1);
		SlotWidget_1->UpdateSlot(nullptr);
	}
	if (SlotWidget_2)
	{
		SlotWidget_2->InitSlot(2);
		SlotWidget_2->UpdateSlot(nullptr);
	}

	for (int32 i = 0; i < MAX_PASSIVE_SLOTS; ++i)
	{
		UPassiveSlotWidget* PassiveSlot = GetPassiveSlotWidget(i);
		if (PassiveSlot)
		{
			PassiveSlot->ClearSlot();
		}
	}
}

UWeaponSlotWidget* USlotContainerWidget::GetWeaponSlotWidget(int32 SlotIndex) const
{
	switch (SlotIndex)
	{
	case 0: return SlotWidget_0;
	case 1: return SlotWidget_1;
	case 2: return SlotWidget_2;
	default:
		KHS_WARN("[SlotContainer] 유효하지 않은 WeaponSlotIndex: %d", SlotIndex);
		return nullptr;
	}
}

UCharacterSkillSlotWidget* USlotContainerWidget::GetSkillSlotWidget(int32 SlotIndex) const
{
	switch (SlotIndex)
	{
	case 0: return SkillSlotWidget_0;
	case 1: return SkillSlotWidget_1;
	case 2: return SkillSlotWidget_2;
	case 3: return SkillSlotWidget_3;
	case 4: return SkillSlotWidget_4;
	case 5: return SkillSlotWidget_5;
	default:
		KHS_WARN("[SlotContainer] 유효하지 않은 SkillSlotIndex: %d", SlotIndex);
		return nullptr;
	}
}

UPassiveSlotWidget* USlotContainerWidget::GetPassiveSlotWidget(int32 SlotIndex) const
{
	switch (SlotIndex)
	{
	case 0: return PassiveSlotWidget_0;
	case 1: return PassiveSlotWidget_1;
	case 2: return PassiveSlotWidget_2;
	case 3: return PassiveSlotWidget_3;
	case 4: return PassiveSlotWidget_4;
	case 5: return PassiveSlotWidget_5;
	case 6: return PassiveSlotWidget_6;
	case 7: return PassiveSlotWidget_7;
	default:
		KHS_WARN("[SlotContainer] 유효하지 않은 PassiveSlotIndex: %d", SlotIndex);
		return nullptr;
	}
}

void USlotContainerWidget::UpdatePassiveSlots(const TArray<FName>& EquippedPassiveIDs)
{
	for (int32 i = 0; i < MAX_PASSIVE_SLOTS; ++i)
	{
		UPassiveSlotWidget* PassiveSlot = GetPassiveSlotWidget(i);
		if (!PassiveSlot)
		{
			continue;
		}

		if (i < EquippedPassiveIDs.Num())
		{
			PassiveSlot->UpdateSlot(EquippedPassiveIDs[i]);
		}
		else
		{
			PassiveSlot->ClearSlot();
		}
	}
}
