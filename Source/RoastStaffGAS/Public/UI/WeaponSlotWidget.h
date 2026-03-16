// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WeaponSlotWidget.generated.h"

/**
 * 하단 무기 슬롯별 정보를 담당하는 UI클래스
 */

struct FWeaponSlotInstanceData;
class UMaterialInstanceDynamic;
class UTextBlock;
class UImage;


UCLASS()
class ROASTSTAFFGAS_API UWeaponSlotWidget : public UUserWidget
{
	GENERATED_BODY()
	
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
	TObjectPtr<UImage> Img_SkillIcon;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_CooldownRemaining;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img_ActiveBorder;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> CooldownMID;
	UPROPERTY()
	TObjectPtr<UTexture2D> LoadedSkillIcon;


	int32 SlotIndex = -1;
	float TotalCooldown = 0.f;
	float LocalCooldownRemaining = 0.f;
	bool bIsCooldownActive = false;
	static const FName CooldownPercentParam;
	
};
