// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/RSBaseWidget.h"
#include "Data/DataTableStructs.h"
#include "RSStageSelectWidget.generated.h"

class UButton;
class UImage;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStageSelected, FName, StageID);

/**
 * URSStageSelectWidget
 *
 * 스테이지 선택 50/50 분할 카드 PAGE 위젯 (v2.0).
 * NativeConstruct에서 GDS 조회 → 좌/우 카드에 스테이지 정보 세팅.
 * 카드 클릭 → 즉시 OnStageSelectedDel 브로드캐스트 (Confirm 단계 없음).
 * 호버 확장 애니메이션(60/40)은 WBP WidgetAnimation에서 처리.
 */
UCLASS()
class ROASTSTAFFGAS_API URSStageSelectWidget : public URSBaseWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;

public:
	/** 스테이지 진입 확정 — ARSOutGamePlayerController::OnStageSelected() 바인딩 대상 */
	UPROPERTY(BlueprintAssignable, Category = "RS|StageSelect")
	FOnStageSelected OnStageSelectedDel;

private:
	/** GDS에서 스테이지 2개 조회 → 좌/우 카드 세팅 */
	void InitStageCards();

	/** 카드 1개분 이미지 + 텍스트 세팅 */
	void SetStageCardDisplay(UImage* Img, UTextBlock* Txt, const FStageStaticData& Data);

	UFUNCTION() void OnCardLeftClicked();
	UFUNCTION() void OnCardRightClicked();
	UFUNCTION() void OnBackClicked();

	// ── BindWidget ──────────────────────────────────────────────────────────
	/** 좌측 카드 클릭 감지 (투명 UButton — WBP에서 전체 영역 커버) */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Card_Left;

	/** 우측 카드 클릭 감지 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Card_Right;

	/** 좌측 카드 배경 스크린샷 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img_Card_Left;

	/** 우측 카드 배경 스크린샷 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img_Card_Right;

	/** 좌측 스테이지 이름 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_Stage_Left;

	/** 우측 스테이지 이름 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_Stage_Right;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Back;

	// ── 런타임 상태 ─────────────────────────────────────────────────────────
	/** InitStageCards에서 확정 — OnCardLeftClicked에서 브로드캐스트 */
	FName StageIDLeft = NAME_None;

	/** InitStageCards에서 확정 — OnCardRightClicked에서 브로드캐스트 */
	FName StageIDRight = NAME_None;
};
