// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/RSBaseWidget.h"
#include "Data/RuntimeDataStructs.h"
#include "LevelUpWeaponSelectWidget.generated.h"

class UButton;
class UImage;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeaponSelectCompleted);

/**
 * 레벨업 무기 선택 팝업 위젯
 * - PlayerController가 SetCandidates()로 카드 데이터 주입
 * - Btn_Confirm{i} 클릭 → 해당 카드 무기 즉시 장착 + 닫기
 * - Btn_Select{i}  클릭 → 카드 하이라이트 (SelectedCardIndex 갱신)
 * - Btn_Close      클릭 → 무기 미선택 닫기
 * - 선택 완료/스킵 시 OnWeaponSelectCompletedDel 발행 → PlayerController 게임 재개
 */
UCLASS()
class ROASTSTAFFGAS_API ULevelUpWeaponSelectWidget : public URSBaseWidget
{
	GENERATED_BODY()

public:
	ULevelUpWeaponSelectWidget();

	/** PlayerController에서 카드 표시 데이터를 주입 — 주입 즉시 UI 갱신 */
	void SetCandidates(const TArray<FWeaponCardDisplayData>& InCandidates);

	/** 선택 완료(확인/스킵) 시 발행 — PlayerController가 구독하여 게임 재개 */
	UPROPERTY(BlueprintAssignable, Category = "MY|LevelUp")
	FOnWeaponSelectCompleted OnWeaponSelectCompletedDel;

protected:
	virtual void NativeConstruct() override;
	/** UIManagerSubsystem이 위젯을 재표시할 때마다 선택 상태 초기화 */
	virtual void OpenUI() override;

private:
	// ── 내부 로직 ─────────────────────────────────────────────────────
	/** Btn_Select{i} — 카드 하이라이트 상태만 갱신 (장착 없음) */
	void OnCardSelected(int32 CardIndex);
	/** Btn_Confirm{i} — CardIndex 카드 즉시 장착 후 닫기 */
	void EquipAndClose(int32 CardIndex);
	/** Btn_Close — 무기 미선택 닫기 */
	void CloseWithoutEquip();
	/** SetCandidates 호출 후 텍스트 위젯 일괄 갱신 */
	void RefreshCandidateUI();

	// ── 버튼 OnClicked 핸들러 (AddDynamic — 파라미터 없는 UFUNCTION 필요) ──
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
	UFUNCTION() 
	void OnBtn_CloseClicked();

	// ── BindWidget ─────────────
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> Btn_Select1;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> Btn_Select2;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> Btn_Select3;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> Btn_Confirm1;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> Btn_Confirm2;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> Btn_Confirm3;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> Btn_Close;

	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> Txt_WeaponName1;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> Txt_WeaponName2;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> Txt_WeaponName3;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> Txt_Desc1;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> Txt_Desc2;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> Txt_Desc3;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> Txt_WeaponLevel1;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> Txt_WeaponLevel2;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> Txt_WeaponLevel3;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> Txt_Explain;

	UPROPERTY(meta = (BindWidget)) TObjectPtr<UImage>     Img_WeaponIcon1;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UImage>     Img_WeaponIcon2;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UImage>     Img_WeaponIcon3;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UImage>     Img_Highlight1;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UImage>     Img_Highlight2;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UImage>     Img_Highlight3;

	TArray<FWeaponCardDisplayData> Candidates;
	int32 SelectedCardIndex = -1;
};
