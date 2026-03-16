// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/RSPlayerController.h"
#include "RoastStaffGAS.h"
#include "Subsystems/EquipmentSubsystem.h"
#include "Subsystems/UIManagerSubsystem.h"

#include "UI/RSHUDWidget.h"
#include "UI/WeaponSlotContainerWidget.h"
#include "UI/WeaponSlotWidget.h"
#include "Data/RuntimeDataStructs.h"

void ARSPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	GET_GI_SUBSYSTEM_FROM(UUIManagerSubsystem, UMS, GetGameInstance());
	GET_GI_SUBSYSTEM_FROM(UEquipmentSubsystem, EquipSys, GetGameInstance());
	
	EquipSys->OnSlotUpdatedDel.AddDynamic(this, &ARSPlayerController::OnSlotUpdated);
	KHS_INFO(TEXT("OnSlotUpdatedDel 구독 완료 "));

	if (HUDWidgetClass)
	{
		CachedHUDUI = UMS->OpenUI<URSHUDWidget>(HUDWidgetClass);
	}
	else
	{
		KHS_WARN(TEXT("HUDWidgetClass BP 미할당"));
	}
}

void ARSPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GET_GI_SUBSYSTEM_FROM(UEquipmentSubsystem, EquipSys, GetGameInstance());
	EquipSys->OnSlotUpdatedDel.RemoveDynamic(this, &ARSPlayerController::OnSlotUpdated);
	
	Super::EndPlay(EndPlayReason);
}

void ARSPlayerController::OnSlotUpdated(int32 SlotIndex)
{
	RefreshSlotUI(SlotIndex);
}

void ARSPlayerController::RefreshSlotUI(int32 SlotIndex)
{
	GET_GI_SUBSYSTEM_FROM(UUIManagerSubsystem, UMS, GetGameInstance());
	GET_GI_SUBSYSTEM_FROM(UEquipmentSubsystem, EquipSys, GetGameInstance());

	if (!HUDWidgetClass)
	{
		KHS_WARN(TEXT("HUD WBP 미할당"));
		return;
	}

	CachedHUDUI = UMS->GetOrCreateWidget<URSHUDWidget>(HUDWidgetClass);
	if (!CachedHUDUI)
	{
		KHS_WARN(TEXT("HUD Widget 생성 실패"));
		return;
	}

	UWeaponSlotContainerWidget* SlotContainer = CachedHUDUI->GetSlotContainerWidget();
	if (!SlotContainer)
	{
		KHS_WARN(TEXT("WeaponSlotContainer 획득 실패"));
		return;
	}

	UWeaponSlotWidget* Slot = SlotContainer->GetSlotWidget(SlotIndex);
	if (!Slot)
	{
		KHS_WARN(TEXT("SlotWidget 획득 실패: %d"), SlotIndex);
		return;
	}

	const FWeaponSlotInstanceData* SlotData = EquipSys->GetSlotData(SlotIndex);
	Slot->UpdateSlot(SlotData);

	KHS_INFO(TEXT("Slot %d UI 갱신 — WeaponID: %s"), SlotIndex, SlotData ? *SlotData->EquipData.WeaponID.ToString() : TEXT("null"));
}
