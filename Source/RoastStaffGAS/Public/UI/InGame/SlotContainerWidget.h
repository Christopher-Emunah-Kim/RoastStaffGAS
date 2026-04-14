// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/RSBaseWidget.h"
#include "SlotContainerWidget.generated.h"

class UCharacterSkillSlotWidget;
class UWeaponSlotWidget;

/**
 * 인게임 슬롯 컨테이너 — 캐릭터 스킬(Q/E) + 무기 슬롯 3개 통합
 * 배치 순서: SkillSlotWidget_0(Q) / SkillSlotWidget_1(E) / SlotWidget_0~2(무기)
 */
UCLASS()
class ROASTSTAFFGAS_API USlotContainerWidget : public URSBaseWidget
{
	GENERATED_BODY()

public:
	UWeaponSlotWidget*         GetWeaponSlotWidget(int32 SlotIndex) const;
	UCharacterSkillSlotWidget* GetSkillSlotWidget(int32 SlotIndex) const;

protected:
	virtual void NativeConstruct() override;

private:
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
};
