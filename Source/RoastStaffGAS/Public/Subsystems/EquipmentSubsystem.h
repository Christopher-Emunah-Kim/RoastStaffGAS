// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayAbilitySpec.h"
#include "Data/RuntimeDataStructs.h"
#include "EquipmentSubsystem.generated.h"


// 슬롯 상태 변경 시 발행 — EquipmentComponent(UI)가 구독
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSlotUpdated, int32, SlotIndex);

class UAbilitySystemComponent;
class URSSkillData;

UCLASS()
class ROASTSTAFFGAS_API UEquipmentSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// PlayerCharacter에서 호출
	void InitializeSubsystem(UAbilitySystemComponent* InASC);
	void RequestManualFire(const FVector& AimLocation);
	void RequestSlotActivate(int32 SlotIndex);
	
	// LevelUpSubsystem에서 호출
	void EquipWeapon(const FName& WeaponID);
	
	void StopAllFire();
	// 슬롯 데이터 읽기 UI용
	const FWeaponSlotInstanceData* GetSlotData(int32 SlotIndex) const;

private:
	void FireSlot(int32 SlotIndex, const FVector& AimLocation);
	void StartAutoFire(int32 SlotIndex);
	void StopAutoFire(int32 SlotIndex);
	void SetSlotActive(int32 SlotIndex);
	void ClearActiveSlot();

	bool IsValidSlotIndex(int32 SlotIndex) const;
	int32 GetEmptySlotIndex() const;
	FGameplayTag GetEventTag(const FWeaponSlotInstanceData& Slot) const;

public:
	// 슬롯 상태 변경 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "MY|Equipment")
	FOnSlotUpdated OnSlotUpdatedDel;
	
private:
	static constexpr int32 SLOT_COUNT = 3;
	
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> ASC;
	UPROPERTY()
	TArray<TObjectPtr<URSSkillData>> SkillDataObjects; // GC 방지

	FWeaponSlotInstanceData Slots[SLOT_COUNT];
	int32 ActiveSlotIndex = -1;

	bool bIsInitialized = false;
};
