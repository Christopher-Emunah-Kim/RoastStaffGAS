// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/RSBaseWidget.h"
#include "CharacterStatPopupWidget.generated.h"

class UTextBlock;
class UButton;
class UAbilitySystemComponent;
class UBaseAttributeSet;
class UPlayerAttributeSet;

/**
 * 캐릭터 스탯 팝업 위젯
 * - HUD 자식 위젯으로 동작 (UILayer=NONE, UMS 미등록)
 * - 입력 모드 변경 없음 — 게임 진행 비방해
 * - OpenUI 시 AttributeSet 델리게이트 구독, CloseUI 시 언바인딩
 */
UCLASS()
class ROASTSTAFFGAS_API UCharacterStatPopupWidget : public URSBaseWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	virtual void OpenUI() override;
	virtual void CloseUI() override;

private:
	// 버튼 핸들러
	UFUNCTION()
	void OnCloseClicked();

	// 델리게이트 핸들러
	UFUNCTION()
	void OnPlayerStatChanged(float NewATK, float NewDEF, float NewAttackSpeed, float NewCritRate, float NewCritDmg);
	UFUNCTION()
	void OnMoveSpeedChanged(float NewValue);
	UFUNCTION()
	void OnHealthChanged(float NewHP, float NewMaxHP);

	// 현재 값으로 TextBlock 전체 갱신
	void RefreshAllStats();

private:
	// BindWidget — Btn
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Close;

	// BindWidget — TextBlock (스탯 수치)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_ATK;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_DEF;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_HP;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_MoveSpeed;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_CritRate;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_CritDmg;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_AttackSpeed;

	// ASC / AttributeSet 캐시 (GC 추적)
	UPROPERTY()
	TWeakObjectPtr<UBaseAttributeSet> CachedBaseAS;
	UPROPERTY()
	TWeakObjectPtr<UPlayerAttributeSet> CachedPlayerAS;
};
