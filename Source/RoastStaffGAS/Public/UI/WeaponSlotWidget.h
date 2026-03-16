// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/RSBaseWidget.h"
#include "WeaponSlotWidget.generated.h"

/**
 * 하단 무기 슬롯별 정보를 담당하는 UI클래스
 */

struct FWeaponSlotInstanceData;
class UTextBlock;
class UImage;


UCLASS()
class ROASTSTAFFGAS_API UWeaponSlotWidget : public UUserWidget
{
	GENERATED_BODY()
	
	UWeaponSlotWidget();
	
public:
	void InitSlot(int32 InSlotIndex);
	void UpdateSlot(const FWeaponSlotInstanceData* SlotData);
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
	void UpdateCooldown(float InDeltaTime);
	
private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_WeaponName;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img_CooldownOverlay;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_CooldownRemaining;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img_ActiveBorder;

	int32 SlotIndex = -1;
	float LocalCooldownRemaining = 0.f;
	bool bIsCooldownActive = false;
	
};
