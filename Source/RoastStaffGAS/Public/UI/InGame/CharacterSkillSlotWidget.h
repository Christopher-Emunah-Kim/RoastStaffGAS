// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterSkillSlotWidget.generated.h"

struct FSkillSlotState;
class UMaterialInstanceDynamic;
class UTextBlock;
class UImage;
class UTexture2D;

/**
 * 캐릭터 고유 스킬 슬롯 UI (Q/E)
 * WeaponSlotWidget과 동일한 쿨타임 패턴 사용 (NativeTick 로컬 감소)
 */
UCLASS()
class ROASTSTAFFGAS_API UCharacterSkillSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitSlot(int32 InSlotIndex);
	/** 슬롯 상태 갱신. SlotState == nullptr or SkillID == None 이면 Collapsed */
	void UpdateSlot(const FSkillSlotState* SlotState);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	void UpdateCooldown(float InDeltaTime);

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img_SkillIcon;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img_CooldownOverlay;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_CooldownRemaining;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_SkillName;

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
