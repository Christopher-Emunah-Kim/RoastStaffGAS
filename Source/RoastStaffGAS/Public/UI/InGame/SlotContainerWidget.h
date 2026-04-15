// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/RSBaseWidget.h"
#include "SlotContainerWidget.generated.h"

class UCharacterSkillSlotWidget;
class UPassiveSlotWidget;
class UWeaponSlotWidget;

/**
 * 인게임 슬롯 컨테이너 — 캐릭터 스킬(Q/E) + 무기 슬롯 + 패시브 슬롯 통합
 * 배치 순서: SkillSlotWidget_0(Q) / SkillSlotWidget_1(E) / SlotWidget_0~1(무기) / PassiveSlotWidget_0~7(패시브)
 */
UCLASS()
class ROASTSTAFFGAS_API USlotContainerWidget : public URSBaseWidget
{
	GENERATED_BODY()

public:
	UWeaponSlotWidget*         GetWeaponSlotWidget(int32 SlotIndex) const;
	UCharacterSkillSlotWidget* GetSkillSlotWidget(int32 SlotIndex) const;
	UPassiveSlotWidget*        GetPassiveSlotWidget(int32 SlotIndex) const;

	/** 패시브 슬롯 전체 갱신 — index < EquippedNum: UpdateSlot / 나머지: ClearSlot */
	void UpdatePassiveSlots(const TArray<FName>& EquippedPassiveIDs);

protected:
	virtual void NativeConstruct() override;

private:
	static constexpr int32 MAX_PASSIVE_SLOTS = 8;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCharacterSkillSlotWidget> SkillSlotWidget_0;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCharacterSkillSlotWidget> SkillSlotWidget_1;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWeaponSlotWidget> SlotWidget_0;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWeaponSlotWidget> SlotWidget_1;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWeaponSlotWidget> SlotWidget_2;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPassiveSlotWidget> PassiveSlotWidget_0;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPassiveSlotWidget> PassiveSlotWidget_1;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPassiveSlotWidget> PassiveSlotWidget_2;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPassiveSlotWidget> PassiveSlotWidget_3;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPassiveSlotWidget> PassiveSlotWidget_4;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPassiveSlotWidget> PassiveSlotWidget_5;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPassiveSlotWidget> PassiveSlotWidget_6;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPassiveSlotWidget> PassiveSlotWidget_7;
};
