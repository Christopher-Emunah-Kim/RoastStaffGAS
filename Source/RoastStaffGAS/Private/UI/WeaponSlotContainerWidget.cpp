// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WeaponSlotContainerWidget.h"

#include "System/LoggingSystem.h"
#include "UI/WeaponSlotWidget.h"


void UWeaponSlotContainerWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	ensureMsgf(SlotWidget_0, TEXT("SlotWidget_0 BindWidget 누락"));
	ensureMsgf(SlotWidget_1, TEXT("SlotWidget_1 BindWidget 누락"));
	ensureMsgf(SlotWidget_2, TEXT("SlotWidget_2 BindWidget 누락"));

	SlotWidget_0->InitSlot(0);
	SlotWidget_1->InitSlot(1);
	SlotWidget_2->InitSlot(2);

}

UWeaponSlotWidget* UWeaponSlotContainerWidget::GetSlotWidget(int32 SlotIndex) const
{
	switch (SlotIndex)
	{
	case 0: return SlotWidget_0;
	case 1: return SlotWidget_1;
	case 2: return SlotWidget_2;
	default:
		{
			KHS_WARN(TEXT("[SlotContainer] 유효하지 않은 SlotIndex: %d"), SlotIndex);
			return nullptr;
		}
	}
}