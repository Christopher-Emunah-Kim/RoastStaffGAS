// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/EquipmentComponent.h"
#include "Subsystems/EquipmentSubsystem.h"
#include "RoastStaffGAS.h"
#include "Data/RuntimeDataStructs.h"
#include "System/LoggingSystem.h"

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
	GET_GI_SUBSYSTEM_FROM(UEquipmentSubsystem, EquipSys, GetWorld()->GetGameInstance());

	const FWeaponSlotInstanceData* SlotData = EquipSys->GetSlotData(SlotIndex);
	if (!SlotData)
	{
		return;
	}

	// TODO: 슬롯 UI 위젯에 SlotData 전달하여 갱신
	// SlotWidgets[SlotIndex]->UpdateDisplay(*SlotData);
	KHS_INFO(TEXT("Slot %d UI 갱신 — WeaponID: %s"),	SlotIndex, *SlotData->EquipData.WeaponID.ToString());
}

