// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/RSBaseWidget.h"
#include "RSHUDWidget.generated.h"


class USlotContainerWidget;
class UCharacterStatPopupWidget;
class UBossHPBarWidget;
class UAbilitySystemComponent;
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

	/** 보스 스폰 시 EnemySpawner가 호출 — BossHPBar 표시 + ASC 바인딩 */
	void ShowBossHPBar(UAbilitySystemComponent* InASC, float InPhase2Ratio);
	/** 보스 사망 폴백 시 EnemySpawner가 호출 — BossHPBar 숨김 */
	void HideBossHPBar();

	FORCEINLINE UBossHPBarWidget* GetBossHPBar() const { return WBP_BossHPBar; }

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

	/** 보스 전투 중에만 표시 — 초기 Collapsed, 보스 스폰/사망 시 토글 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBossHPBarWidget> WBP_BossHPBar;
};
