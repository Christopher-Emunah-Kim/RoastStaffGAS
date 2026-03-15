// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EquipmentComponent.generated.h"

// -------------------------------------------------------------------------
// UEquipmentComponent
// 슬롯UI 표현 전담
// -------------------------------------------------------------------------

class UEquipmentSubsystem;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ROASTSTAFFGAS_API UEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()
	
public:
	UEquipmentComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;	

private:
	// EquipmentSubsystem::OnSlotUpdatedDel 수신 → 해당 슬롯 UI 갱신
	UFUNCTION()
	void OnSlotUpdated(int32 SlotIndex);

	// TODO: 슬롯 UI 위젯 연결 후 실제 갱신 로직 구현
	void RefreshSlotUI(int32 SlotIndex);
	
};
