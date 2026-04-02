// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/RSBaseWidget.h"
#include "Data/DataTableStructs.h"
#include "Data/EnumTypes.h"
#include "RSStageNodeWidget.generated.h"

class UButton;
class UImage;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStageNodeClicked, FName, StageID);

/**
 * URSStageNodeWidget
 *
 * 스테이지 선택 노드맵의 개별 노드 카드 위젯.
 * PopulateNodeMap에서 동적 생성 후 SetNodeState로 초기화.
 *
 */
UCLASS()
class ROASTSTAFFGAS_API URSStageNodeWidget : public URSBaseWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;

public:
	/** 스테이지 데이터와 노드 상태를 적용 — PopulateNodeMap에서 호출 */
	void SetNodeState(const FStageStaticData& Data, EStageNodeState State);

	FORCEINLINE FName GetStageID() const { return CachedStageID; }

protected:
	UFUNCTION() void OnBtnNodeClicked();

public:
	/** 노드 클릭 시 부모 위젯으로 StageID 전달 (LOCKED 시 발행 안 됨) */
	UPROPERTY(BlueprintAssignable, Category = "RS|StageNode")
	FOnStageNodeClicked OnStageNodeClickedDel;

protected:
	/** 노드 전체 클릭 영역 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Node;
	/** 스테이지 썸네일 배경 이미지 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img_Thumbnail;
	/** 보스 스테이지 아이콘 — bIsBoss=true 시 HitTestInvisible, 기본 Collapsed */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img_BossIcon;
	/** 잠금 오버레이 — LOCKED 시 HitTestInvisible, 기본 Collapsed */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img_LockIcon;
	/** 클리어 체크마크 — CLEARED 시 HitTestInvisible, 기본 Collapsed */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img_ClearMark;
	/** 스테이지 DisplayName */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_NodeName;

private:
	FName CachedStageID = NAME_None;
};
