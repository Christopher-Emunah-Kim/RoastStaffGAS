// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayAbilitySpec.h"
#include "Data/RuntimeDataStructs.h"
#include "EquipmentComponent.generated.h"

// -------------------------------------------------------------------------
// UEquipmentComponent
// RSPlayerCharacter에 부착. 무기 슬롯 3개(0,1,2)를 관리한다.
// -------------------------------------------------------------------------

class UAbilitySystemComponent;
class UGameplayAbility;
class UGameplayEffect;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ROASTSTAFFGAS_API UEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()
	
public:
	UEquipmentComponent();

protected:
	virtual void BeginPlay() override;

private:
	// 슬롯 내부 발사 처리
	void FireSlot(int32 SlotIndex);
	// 자동공격 타이머 시작/중지
	void StartAutoFire(int32 SlotIndex);
	void StopAutoFire(int32 SlotIndex);
	// 액티브 슬롯 전환
	void SetSlotActive(int32 SlotIndex);
	void ClearActiveSlot();
	// 현재 에임 월드 좌표 계산
	FVector GetAimWorldLocation() const;
	// 슬롯 인덱스 유효성
	bool IsValidSlotIndex(int32 SlotIndex) const;

public:
	// PlayerCharacter 초기화 시 ASC 주입
	void InitializeWithASC(UAbilitySystemComponent* InASC);
	// 무기 장착 — 레벨업 시 호출
	void EquipWeapon(const FName& WeaponID);
	// 슬롯 액티브 모드 전환 — Num1/2/3 입력 시 호출
	void RequestSlotActivate(int32 SlotIndex);
	// 수동 발사 — 마우스 클릭 시 호출 (액티브 슬롯만)
	void OnAttackInput();
	
private:
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> ASC;
	
	static constexpr int32 SLOT_COUNT = 3;
	FWeaponSlotInstanceData Slots[SLOT_COUNT];
	int32 ActiveSlotIndex = -1;
};
