// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/OutGame/RSStageSelectWidget.h"
#include "Subsystems/UIManagerSubsystem.h"
#include "Subsystems/GameDataSubsystem.h"
#include "Subsystems/SaveGameSubsystem.h"
#include "System/LoggingSystem.h"
#include "RoastStaffGAS.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"

void URSStageSelectWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// 버튼 바인딩 — NativeOnInitialized에 배치(풀링 재사용 시 중복 방지)
	if (Btn_Back)
	{
		Btn_Back->OnClicked.AddDynamic(this, &URSStageSelectWidget::OnBackClicked);
	}

	if (Btn_Confirm)
	{
		Btn_Confirm->OnClicked.AddDynamic(this, &URSStageSelectWidget::OnConfirmClicked);
	}
}

void URSStageSelectWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 미선택 상태로 초기화 후 노드맵 구성
	SelectedStageID = NAME_None;
	if (Btn_Confirm)
	{
		Btn_Confirm->SetIsEnabled(false);
	}
	ClearDetailPanel();
	PopulateNodeMap();
}

void URSStageSelectWidget::NativeDestruct()
{
	// 동적 생성 노드 위젯의 델리게이트 해제 — GC 전 수동 해제 필요
	for (URSStageNodeWidget* Node : NodeWidgets)
	{
		if (Node)
		{
			Node->OnStageNodeClickedDel.RemoveDynamic(this, &URSStageSelectWidget::OnNodeClicked);
		}
	}
	NodeWidgets.Empty();
	NodeDataCache.Empty();

	Super::NativeDestruct();
}

// -----------------------------------------------------------------------------

void URSStageSelectWidget::PopulateNodeMap()
{
	if (!NodeMapContainer)
	{
		KHS_WARN(TEXT("URSStageSelectWidget::PopulateNodeMap — NodeMapContainer가 null입니다."));
		return;
	}

	if (!NodeWidgetClass)
	{
		KHS_WARN(TEXT("URSStageSelectWidget::PopulateNodeMap — NodeWidgetClass가 설정되지 않았습니다."));
		return;
	}

	GET_GI_SUBSYSTEM_FROM(UGameDataSubsystem, GDS, GetWorld()->GetGameInstance());
	GET_GI_SUBSYSTEM_FROM(USaveGameSubsystem, SGS, GetWorld()->GetGameInstance());

	TArray<FStageStaticData> AllStages;
	if (!GDS->GetAllStageStaticData(AllStages))
	{
		KHS_WARN(TEXT("URSStageSelectWidget::PopulateNodeMap — 스테이지 데이터 조회 실패."));
		return;
	}

	NodeMapContainer->ClearChildren();
	NodeWidgets.Empty();
	NodeDataCache.Empty();

	for (const FStageStaticData& StageData : AllStages)
	{
		// AVAILABLE / CLEARED / LOCKED 판정
		// UnlockStageID == None → 첫 스테이지(항상 진입 가능)
		EStageNodeState State;
		if (SGS->IsStageCleared(StageData.StageID))
		{
			State = EStageNodeState::CLEARED;
		}
		else if (StageData.UnlockStageID.IsNone() || SGS->IsStageCleared(StageData.UnlockStageID))
		{
			State = EStageNodeState::AVAILABLE;
		}
		else
		{
			State = EStageNodeState::LOCKED;
		}

		URSStageNodeWidget* Node = CreateWidget<URSStageNodeWidget>(this, NodeWidgetClass);
		if (!Node)
		{
			continue;
		}

		Node->SetNodeState(StageData, State);

		// 중복 바인딩 방지 후 클릭 이벤트 구독
		Node->OnStageNodeClickedDel.RemoveDynamic(this, &URSStageSelectWidget::OnNodeClicked);
		Node->OnStageNodeClickedDel.AddDynamic(this, &URSStageSelectWidget::OnNodeClicked);

		NodeMapContainer->AddChild(Node);
		NodeWidgets.Add(Node);
		NodeDataCache.Add(StageData.StageID, TPair<FStageStaticData, EStageNodeState>(StageData, State));
	}
}

// ── 버튼 핸들러 ───────────────────────────────────────────────────────────────

void URSStageSelectWidget::OnNodeClicked(FName StageID)
{
	const TPair<FStageStaticData, EStageNodeState>* Found = NodeDataCache.Find(StageID);
	if (!Found)
	{
		KHS_WARN(TEXT("URSStageSelectWidget::OnNodeClicked — StageID '%s'가 캐시에 없습니다."), *StageID.ToString());
		return;
	}

	SelectedStageID = StageID;
	UpdateDetailPanel(Found->Key, Found->Value);

	// LOCKED 노드는 SetNodeState에서 Btn_Node가 disabled이므로 도달하지 않지만 방어 처리
	if (Btn_Confirm)
	{
		Btn_Confirm->SetIsEnabled(Found->Value != EStageNodeState::LOCKED);
	}
}

void URSStageSelectWidget::OnConfirmClicked()
{
	if (SelectedStageID.IsNone())
	{
		KHS_WARN(TEXT("URSStageSelectWidget::OnConfirmClicked — 선택된 스테이지가 없습니다."));
		return;
	}

	OnStageSelectedDel.Broadcast(SelectedStageID);
}

void URSStageSelectWidget::OnBackClicked()
{
	GET_GI_SUBSYSTEM_FROM(UUIManagerSubsystem, UMS, GetWorld()->GetGameInstance());
	UMS->BackPage();
}

// ── 상세 패널 ─────────────────────────────────────────────────────────────────

void URSStageSelectWidget::UpdateDetailPanel(const FStageStaticData& Data, EStageNodeState State)
{
	if (Txt_SelectedName)
	{
		Txt_SelectedName->SetText(Data.DisplayName);
	}

	if (Txt_TimeLimit)
	{
		// 정수 초 단위로 표시 (예: "120s")
		Txt_TimeLimit->SetText(FText::FromString(FString::Printf(TEXT("%.0fs"), Data.TimeLimit)));
	}

	if (Txt_NodeStatus)
	{
		FText StatusText;
		switch (State)
		{
		case EStageNodeState::AVAILABLE: StatusText = FText::FromString(TEXT("AVAILABLE")); break;
		case EStageNodeState::CLEARED:   StatusText = FText::FromString(TEXT("CLEARED"));   break;
		case EStageNodeState::LOCKED:    StatusText = FText::FromString(TEXT("LOCKED"));    break;
		default:                         StatusText = FText::GetEmpty();                    break;
		}
		Txt_NodeStatus->SetText(StatusText);
	}

	if (Img_SelectedThumb && !Data.Thumbnail.IsNull())
	{
		if (UTexture2D* Tex = Data.Thumbnail.LoadSynchronous())
		{
			Img_SelectedThumb->SetBrushFromTexture(Tex, true);
		}
	}
}

void URSStageSelectWidget::ClearDetailPanel()
{
	const FText EmptyText = FText::GetEmpty();

	if (Txt_SelectedName)  { Txt_SelectedName->SetText(EmptyText);  }
	if (Txt_TimeLimit)     { Txt_TimeLimit->SetText(EmptyText);      }
	if (Txt_NodeStatus)    { Txt_NodeStatus->SetText(EmptyText);     }
}
