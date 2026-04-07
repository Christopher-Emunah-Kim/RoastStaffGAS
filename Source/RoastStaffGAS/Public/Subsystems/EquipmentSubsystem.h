// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayAbilitySpec.h"
#include "Data/RuntimeDataStructs.h"
#include "System/LoggingSystem.h"
#include "EquipmentSubsystem.generated.h"


// 슬롯 상태 변경 시 발행 — EquipmentComponent(UI)가 구독
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSlotUpdated, int32, SlotIndex);
// 슬롯 가득 + 강화 불가 시 발행 — PlayerController가 구독해 교체 UI 열기
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponSlotFull, FName, PendingWeaponID);

class UAbilitySystemComponent;
class URSSkillData;

struct FLoadedEquipClasses //EquipWeapon 내부헬퍼
{
	TSubclassOf<UGameplayAbility> GAClass;
};

UCLASS()
class ROASTSTAFFGAS_API UEquipmentSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// PlayerCharacter에서 호출
	void InitializeSubsystem(UAbilitySystemComponent* InASC);
	/** 레벨 전환 직전 호출 — 타이머 전량 정리 + ASC 참조 해제 */
	void DeinitializeSubsystem();
	void RequestManualFire(const FVector& AimLocation);
	void RequestSlotActivate(int32 SlotIndex);
	
	// LevelUpSubsystem에서 호출
	void EquipWeapon(const FName& WeaponID);
	// 동일 슬롯에 다음 레벨 무기 장착 (강화 / 교체 UI 확인 공용)
	void UpgradeWeapon(int32 SlotIndex, FName NextWeaponID);
	
	//공격 중지
	void StopAllFire();
	//GA EndAbility에서 호출(쿨타임 관리)
	void OnSummonAbilityEnded(FGameplayAbilitySpecHandle SpecHandle);
	
	// 슬롯 데이터 읽기 UI용
	const FWeaponSlotInstanceData* GetSlotData(int32 SlotIndex) const;

	FORCEINLINE int32 GetSlotCount() const {return SLOT_COUNT;}
	
private:
	void FireSlot(int32 SlotIndex, const FVector& AimLocation);
	void SetGameplayEventData(const FVector& AimLocation, FGameplayEventData& Payload);
	void StartAutoFire(int32 SlotIndex);
	void StopAutoFire(int32 SlotIndex);
	void SetSlotActive(int32 SlotIndex);
	void ClearActiveSlot();
	
	
	void ClearSlot(int32 SlotIndex);

	bool IsValidSlotIndex(int32 SlotIndex) const;
	int32 GetEmptySlotIndex() const;
	FGameplayTag GetEventTag(const FWeaponSlotInstanceData& Slot) const;

	//EquipWeapon 내부 헬퍼
	bool LoadEquipData(const FName& WeaponID, FWeaponSlotEquipData& OutData) const;
	bool LoadEquipClasses(const FWeaponSlotEquipData& EquipData, FLoadedEquipClasses& OutClasses) const;
	bool RegisterAbility(const FWeaponSlotEquipData& EquipData, const FLoadedEquipClasses& Classes, FGameplayAbilitySpecHandle& OutHandle);
	void CommitSlot(int32 TargetSlot,const FWeaponSlotEquipData& EquipData, const FGameplayAbilitySpecHandle& Handle);
	
	template<typename T>
	bool LoadRequiredClass(const TSoftClassPtr<T>& SoftPtr, TSubclassOf<T>& OutClass, const FName& ContextID) const;
	template<typename T>
	TSubclassOf<T> LoadOptionalClass(const TSoftClassPtr<T>& SoftPtr, const FName& ContextID) const;
	
public:
	// 슬롯 상태 변경 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "MY|Equipment")
	FOnSlotUpdated OnSlotUpdatedDel;

	// 슬롯 가득 + 강화 불가 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "MY|Equipment")
	FOnWeaponSlotFull OnSlotFull;

	FName PendingWeaponID = NAME_None;
	
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

template<typename T>
bool UEquipmentSubsystem::LoadRequiredClass(const TSoftClassPtr<T>& SoftPtr,  TSubclassOf<T>& OutClass, const FName& ContextID) const
{
	OutClass = SoftPtr.LoadSynchronous();
	return ensureMsgf(OutClass,	TEXT("필수 클래스 로드 실패 — ID: %s / Class: %s"),	*ContextID.ToString(), *SoftPtr.ToString());
}

template<typename T>
TSubclassOf<T> UEquipmentSubsystem::LoadOptionalClass(const TSoftClassPtr<T>& SoftPtr, const FName& ContextID) const
{
	if (SoftPtr.IsNull())
	{
		return nullptr;
	}

	TSubclassOf<T> Result = SoftPtr.LoadSynchronous();
	if (!Result)
	{
		KHS_WARN(TEXT("선택 클래스 로드 실패 — 스킵. ID: %s / Class: %s"),*ContextID.ToString(),	*SoftPtr.ToString());
	}

	return Result;
}