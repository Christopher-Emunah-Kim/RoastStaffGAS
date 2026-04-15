// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PassiveSlotWidget.generated.h"

class UButton;
class UImage;
class UOverlay;
class UTextBlock;
class UTexture2D;

/**
 * 패시브 슬롯 1칸 위젯
 * - 기본 상태: Hidden (레이아웃 공간 유지, 비표시)
 * - hover 시 Bdr_Tooltip (DisplayName + Description) 표시
 * - 쿨다운/이름 레이블 없음 — 아이콘 + 툴팁만
 */
UCLASS()
class ROASTSTAFFGAS_API UPassiveSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 패시브 ID로 슬롯 갱신. GDS에서 데이터 조회 후 아이콘·텍스트 세팅 → Visible */
	void UpdateSlot(FName PassiveID);
	/** 슬롯 초기화 — Hidden 전환 */
	void ClearSlot();

protected:
	virtual void NativeOnInitialized() override;

private:
	UFUNCTION()
	void OnSlotHovered();
	UFUNCTION()
	void OnSlotUnhovered();

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_PassiveSlot;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img_PassiveIcon;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UOverlay> Ovl_Tooltip;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_PassiveName;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_PassiveDesc;
	UPROPERTY()
	TObjectPtr<UTexture2D> LoadedPassiveIcon;
};
