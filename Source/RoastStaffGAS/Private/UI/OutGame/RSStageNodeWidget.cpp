// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/OutGame/RSStageNodeWidget.h"
#include "System/LoggingSystem.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"

void URSStageNodeWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// 버튼 바인딩 — NativeOnInitialized에 배치(풀링 재사용 시 중복 방지)
	if (Btn_Node)
	{
		Btn_Node->OnClicked.AddDynamic(this, &URSStageNodeWidget::OnBtnNodeClicked);
	}
}

void URSStageNodeWidget::SetNodeState(const FStageStaticData& Data, EStageNodeState State)
{
	CachedStageID = Data.StageID;

	// 썸네일 — TSoftObjectPtr 동기 로드 (노드 수가 적어 블로킹 허용)
	if (Img_Thumbnail && !Data.Thumbnail.IsNull())
	{
		if (UTexture2D* Tex = Data.Thumbnail.LoadSynchronous())
		{
			Img_Thumbnail->SetBrushFromTexture(Tex, true);
		}
	}

	if (Txt_NodeName)
	{
		Txt_NodeName->SetText(Data.DisplayName);
	}

	// 보스 아이콘 — v2.0 모든 스테이지가 보스 스테이지로 단일화됨, 항상 Collapsed
	if (Img_BossIcon)
	{
		Img_BossIcon->SetVisibility(ESlateVisibility::Collapsed);
	}

	// 상태별 오버레이 표시
	const bool bIsLocked  = (State == EStageNodeState::LOCKED);
	const bool bIsCleared = (State == EStageNodeState::CLEARED);

	if (Img_LockIcon)
	{
		Img_LockIcon->SetVisibility(bIsLocked ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}

	if (Img_ClearMark)
	{
		Img_ClearMark->SetVisibility(bIsCleared ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}

	// LOCKED 노드는 클릭 차단
	if (Btn_Node)
	{
		Btn_Node->SetIsEnabled(!bIsLocked);
	}
}

void URSStageNodeWidget::OnBtnNodeClicked()
{
	if (CachedStageID.IsNone())
	{
		KHS_WARN("RSStageNodeWidget::OnBtnNodeClicked — CachedStageID가 None입니다.");
		return;
	}

	OnStageNodeClickedDel.Broadcast(CachedStageID);
}
