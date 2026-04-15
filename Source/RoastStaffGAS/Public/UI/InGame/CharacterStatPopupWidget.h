// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/RSBaseWidget.h"
#include "CharacterStatPopupWidget.generated.h"

struct FOnAttributeChangeData;
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
	// ASC 어트리뷰트 델리게이트 핸들러 — 변경 경로 무관하게 자동 호출
	void OnStatChanged(const FOnAttributeChangeData& Data);
	// 현재 값으로 TextBlock 전체 갱신
	void RefreshAllStats();
	/** 정수 스탯 표시: 패시브 보너스 없으면 "30", 있으면 "30 (+1)" */
	FText MakeStatText(float Base, float Aggregated) const;
	/** % 스탯 표시: "8%" or "8% (+2%)" */
	FText MakePercentText(float Base, float Aggregated) const;

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

	// 캐시 (GC 추적)
	UPROPERTY()
	TWeakObjectPtr<UAbilitySystemComponent> CachedASC;
	UPROPERTY()
	TWeakObjectPtr<UPlayerAttributeSet> CachedPlayerAS;
};
