// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/RSBaseWidget.h"
#include "RSHUDWidget.generated.h"


class USlotContainerWidget;
class UCharacterStatPopupWidget;
class UButton;

/**
 *
 */
UCLASS()
class ROASTSTAFFGAS_API URSHUDWidget : public URSBaseWidget
{
	GENERATED_BODY()

	URSHUDWidget();

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;

public:
	FORCEINLINE USlotContainerWidget* GetSlotContainerWidget() const { return WBP_SlotContainer; }

	/** Tab 키 또는 HUD 버튼에서 호출 — 스탯 팝업 열기/닫기 토글 */
	void ToggleStatPopup();

private:
	UFUNCTION()
	void OnStatPopupBtnClicked();

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USlotContainerWidget> WBP_SlotContainer;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCharacterStatPopupWidget> WBP_CharacterStatPopup;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_StatPopup;
};
