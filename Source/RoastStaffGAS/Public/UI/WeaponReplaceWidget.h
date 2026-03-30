// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/RSBaseWidget.h"
#include "WeaponReplaceWidget.generated.h"

class UButton;
class UImage;
class UTextBlock;

/** 교체 선택 완료(확인) 시 발행 — PlayerController가 구독하여 게임 재개 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeaponReplaceCompleted);

/**
 * UWeaponReplaceWidget
 * 슬롯 가득 + 강화 불가 시 기존 슬롯 무기를 교체할 슬롯을 선택하는 팝업.
 * - 스킵 불가(닫기 버튼 없음) — 기획서 교체 UI 규칙
 * - Btn_Select{i} 클릭 → 슬롯 하이라이트 + Btn_Confirm{i} 활성화
 * - Btn_Confirm{i} 클릭 → EquipmentSubsystem.UpgradeWeapon 호출 → 닫기
 */
UCLASS()
class ROASTSTAFFGAS_API UWeaponReplaceWidget : public URSBaseWidget
{
	GENERATED_BODY()

public:
	UWeaponReplaceWidget();

	/** PlayerController에서 PendingWeaponID 주입 — 슬롯 데이터는 EQS에서 직접 조회 */
	void InitWidget(FName InPendingWeaponID);

	/** 교체 확인 완료 시 발행 — PlayerController가 구독 */
	UPROPERTY(BlueprintAssignable, Category = "MY|WeaponReplace")
	FOnWeaponReplaceCompleted OnReplaceCompletedDel;

protected:
	virtual void NativeOnInitialized() override;
	virtual void OpenUI() override;

private:
	void OnSlotSelected(int32 SlotIndex);
	void OnConfirmClicked(int32 SlotIndex);
	void RefreshSlotUI();

	UFUNCTION() 
	void OnBtn_Select1Clicked();
	UFUNCTION() 
	void OnBtn_Select2Clicked();
	UFUNCTION() 
	void OnBtn_Select3Clicked();
	UFUNCTION() 
	void OnBtn_Confirm1Clicked();
	UFUNCTION() 
	void OnBtn_Confirm2Clicked();
	UFUNCTION() 
	void OnBtn_Confirm3Clicked();

	// ── BindWidget ─────────────────────────────────────────────────────────
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton>    Btn_Select1;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton>    Btn_Select2;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton>    Btn_Select3;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton>    Btn_Confirm1;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton>    Btn_Confirm2;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton>    Btn_Confirm3;

	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> Txt_WeaponName1;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> Txt_WeaponName2;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> Txt_WeaponName3;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> Txt_WeaponLevel1;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> Txt_WeaponLevel2;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> Txt_WeaponLevel3;

	UPROPERTY(meta = (BindWidget)) TObjectPtr<UImage>     Img_WeaponIcon1;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UImage>     Img_WeaponIcon2;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UImage>     Img_WeaponIcon3;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UImage>     Img_Highlight1;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UImage>     Img_Highlight2;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UImage>     Img_Highlight3;

	FName PendingWeaponID = NAME_None;
	int32 SelectedSlotIndex = -1;
};
