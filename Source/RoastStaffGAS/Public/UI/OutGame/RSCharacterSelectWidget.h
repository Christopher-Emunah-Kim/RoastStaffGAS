// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/RSBaseWidget.h"
#include "UI/OutGame/RSCharacterEntryWidget.h"
#include "RSCharacterSelectWidget.generated.h"

class UScrollBox;
class UButton;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterSelected, FName, CharID);

/**
 * URSCharacterSelectWidget
 *
 * 캐릭터 선택 PAGE 위젯 (캐러셀).
 */
UCLASS()
class ROASTSTAFFGAS_API URSCharacterSelectWidget : public URSBaseWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;

public:
	/** GDS + SGS에서 캐릭터 목록을 조회해 CarouselContainer를 채운다 */
	UFUNCTION(BlueprintCallable, Category = "RS|CharacterSelect")
	void PopulateCarousel();
	/** 그리드 팝업에서 선택된 CharID로 캐러셀 포커스 이동 + 정보 패널 갱신 */
	UFUNCTION(BlueprintCallable, Category = "RS|CharacterSelect")
	void FocusCarouselOn(FName CharID);

protected:
	// ── 버튼 핸들러 ──────────────────────────────────────────────────────────
	UFUNCTION() void OnBackClicked();
	UFUNCTION() void OnGridPopupClicked();
	UFUNCTION() void OnStageSelectClicked();

	// ── 캐러셀 Entry 이벤트 ──────────────────────────────────────────────────
	/** 카드 클릭 → SelectedCharID 갱신 + 정보 패널 갱신 */
	UFUNCTION() void OnCharacterEntryClicked(FName CharID);
	/** 그리드 팝업 포커스 요청 수신 → FocusCarouselOn */
	UFUNCTION() void OnCharacterFocusFromGrid(FName CharID);

	// ── 정보 패널 ────────────────────────────────────────────────────────────
	/** CharID 캐릭터 데이터를 GDS에서 조회해 정보 패널 텍스트에 반영 */
	void UpdateInfoPanel(FName CharID);
	/** 정보 패널 비우기 (캐러셀 재구성 전 초기화용) */
	void ClearInfoPanel();

public:
	/** 선택 확정 — OGPC::OnCharacterSelected() 바인딩 대상 */
	UPROPERTY(BlueprintAssignable, Category = "RS|CharacterSelect")
	FOnCharacterSelected OnCharacterSelectedDel;

protected:
	// -------------------------------------------------------------------------
	// BindWidget — WBP에서 아래 이름과 정확히 일치하는 위젯 생성 필수
	// -------------------------------------------------------------------------
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Back;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_GridPopup;
	/** 캐릭터 카드 가로 스크롤 컨테이너 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> CarouselContainer;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_SelectedName;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_SelectedGrade;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_StatHP;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_StatSpeed;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_StatAttack;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_StageSelect;

	// -------------------------------------------------------------------------
	// 상태
	// -------------------------------------------------------------------------
	/** 현재 정보 패널에 표시 중인 캐릭터 ID */
	FName SelectedCharID = NAME_None;
	/** 생성된 Entry 목록 — FocusCarouselOn 검색용 */
	UPROPERTY()
	TArray<TObjectPtr<URSCharacterEntryWidget>> EntryWidgets;
	/** 생성할 Entry 위젯 클래스 — WBP EditDefaultsOnly로 할당 */
	UPROPERTY(EditDefaultsOnly, Category = "RS|CharacterSelect")
	TSubclassOf<URSCharacterEntryWidget> EntryWidgetClass;
	
};
