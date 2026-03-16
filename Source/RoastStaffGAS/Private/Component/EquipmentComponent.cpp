// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/EquipmentComponent.h"
#include "RoastStaffGAS.h"
#include "Data/RuntimeDataStructs.h"
#include "Subsystems/EquipmentSubsystem.h"
#include "Subsystems/UIManagerSubsystem.h"
#include "System/LoggingSystem.h"
#include "UI/RSHUDWidget.h"
#include "UI/WeaponSlotContainerWidget.h"
#include "UI/WeaponSlotWidget.h"

UEquipmentComponent::UEquipmentComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UEquipmentComponent::BeginPlay()
{
	Super::BeginPlay();
	
	GET_GI_SUBSYSTEM_FROM(UEquipmentSubsystem, EquipSys, GetWorld()->GetGameInstance());
	EquipSys->OnSlotUpdatedDel.AddDynamic(this, &UEquipmentComponent::OnSlotUpdated);
	KHS_INFO(TEXT("OnSlotUpdatedDel 구독 완료"));
	
	if (HUDWidgetClass)
	{
		GET_GI_SUBSYSTEM_FROM(UUIManagerSubsystem, UMS, GetWorld()->GetGameInstance());
		CachedHUDUI = UMS->OpenUI<URSHUDWidget>(HUDWidgetClass);
	}
	else
	{
		KHS_WARN(TEXT("HUDWidgetClass 미설정"));
	}

}

void UEquipmentComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GET_GI_SUBSYSTEM_FROM(UEquipmentSubsystem, EquipSys, GetWorld()->GetGameInstance());
	EquipSys->OnSlotUpdatedDel.RemoveDynamic(this, &UEquipmentComponent::OnSlotUpdated);
	
    Super::EndPlay(EndPlayReason);
}

void UEquipmentComponent::OnSlotUpdated(int32 SlotIndex)
{
	RefreshSlotUI(SlotIndex);
}

void UEquipmentComponent::RefreshSlotUI(int32 SlotIndex)
{
	GET_GI_SUBSYSTEM_FROM(UUIManagerSubsystem, UMS, GetWorld()->GetGameInstance());
	GET_GI_SUBSYSTEM_FROM(UEquipmentSubsystem, EquipSys, GetWorld()->GetGameInstance());
	
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
		KHS_WARN(TEXT("WeaponSlot Contatiner 획득 실패"));
		return;
	}
	
	UWeaponSlotWidget* Slot = SlotContainer->GetSlotWidget(SlotIndex);
	const FWeaponSlotInstanceData* SlotData = EquipSys->GetSlotData(SlotIndex);
	if (!SlotData)
	{
		return;
	}
	
	Slot->UpdateSlot(SlotData);
	KHS_INFO(TEXT("Slot %d UI 갱신 — WeaponID: %s"),	SlotIndex, *SlotData->EquipData.WeaponID.ToString());
}

