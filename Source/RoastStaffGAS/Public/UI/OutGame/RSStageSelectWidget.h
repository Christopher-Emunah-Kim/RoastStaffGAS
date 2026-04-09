// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/RSBaseWidget.h"
#include "UI/OutGame/RSStageNodeWidget.h"
#include "Data/DataTableStructs.h"
#include "RSStageSelectWidget.generated.h"

class UButton;
class UImage;
class UTextBlock;
class UScrollBox;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStageSelected, FName, StageID);

/**
 * URSStageSelectWidget
 *
 * 스테이지 선택 노드맵 PAGE 위젯.
 * CharacterSelectWidget → Btn_StageSelect 클릭으로 진입.
 * 노드 클릭 → 상세 패널 갱신, Btn_Confirm → OnStageSelectedDel 브로드캐스트.
 *
 */
UCLASS()
class ROASTSTAFFGAS_API URSStageSelectWidget : public URSBaseWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

public:
	/** GDS + SGS 조회 후 노드맵을 동적으로 구성 */
	UFUNCTION(BlueprintCallable, Category = "RS|StageSelect")
	void PopulateNodeMap();

protected:
	UFUNCTION() void OnNodeClicked(FName StageID);
	UFUNCTION() void OnConfirmClicked();
	UFUNCTION() void OnBackClicked();

	/** 선택 노드의 상세 패널 갱신 */
	void UpdateDetailPanel(const FStageStaticData& Data, EStageNodeState State);
	/** 상세 패널 텍스트 초기화 */
	void ClearDetailPanel();
	/** SGS의 LastPlayedStageID로 선택 상태 복원 — PopulateNodeMap() 완료 후 호출 */
	void RestoreLastPlayedStage();

public:
	/** 스테이지 진입 확정 — OGPC::OnStageSelected() 바인딩 대상 */
	UPROPERTY(BlueprintAssignable, Category = "RS|StageSelect")
	FOnStageSelected OnStageSelectedDel;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Back;
	/** URSStageNodeWidget을 동적 추가하는 가로 스크롤 컨테이너 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> NodeMapContainer;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_SelectedName;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_TimeLimit;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_NodeStatus;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img_SelectedThumb;
	/** 스테이지 진입 확정 버튼 — 노드 클릭 전까지 disabled */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Confirm;

	// -------------------------------------------------------------------------
	// 에디터 설정
	// -------------------------------------------------------------------------
	/** WBP EditDefaultsOnly로 URSStageNodeWidget 파생 클래스 지정 */
	UPROPERTY(EditDefaultsOnly, Category = "RS|StageSelect")
	TSubclassOf<URSStageNodeWidget> NodeWidgetClass;

private:
	/** 현재 선택된 스테이지 ID (NAME_None = 미선택) */
	FName SelectedStageID = NAME_None;

	/** 생성된 노드 위젯 목록 — NativeDestruct RemoveDynamic 순회용 */
	UPROPERTY()
	TArray<TObjectPtr<URSStageNodeWidget>> NodeWidgets;

	/**
	 * PopulateNodeMap에서 채운 스테이지 데이터 + 상태 캐시.
	 * OnNodeClicked 상세 패널 갱신에 사용.
	 */
	TMap<FName, TPair<FStageStaticData, EStageNodeState>> NodeDataCache;
};
