// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/OutGame/RSStageSelectWidget.h"
#include "Subsystems/UIManagerSubsystem.h"
#include "Subsystems/GameDataSubsystem.h"
#include "System/LoggingSystem.h"
#include "RoastStaffGAS.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"

void URSStageSelectWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (Btn_Card_Left)
	{
		Btn_Card_Left->OnClicked.AddUniqueDynamic(this, &ThisClass::OnCardLeftClicked);
	}

	if (Btn_Card_Right)
	{
		Btn_Card_Right->OnClicked.AddUniqueDynamic(this, &ThisClass::OnCardRightClicked);
	}

	if (Btn_Back)
	{
		Btn_Back->OnClicked.AddUniqueDynamic(this, &ThisClass::OnBackClicked);
	}
}

void URSStageSelectWidget::NativeConstruct()
{
	Super::NativeConstruct();

	StageIDLeft  = NAME_None;
	StageIDRight = NAME_None;

	InitStageCards();
}

// ── 카드 초기화 ───────────────────────────────────────────────────────────────

void URSStageSelectWidget::InitStageCards()
{
	GET_GI_SUBSYSTEM(UGameDataSubsystem, GDS)

	TArray<FStageStaticData> AllStages;
	if (!GDS->GetAllStageStaticData(AllStages))
	{
		KHS_WARN("RSStageSelectWidget::InitStageCards — 스테이지 데이터 조회 실패");
		return;
	}

	if (AllStages.Num() < 2)
	{
		KHS_WARN("RSStageSelectWidget::InitStageCards — 스테이지 데이터가 2개 미만 (%d개)", AllStages.Num());
		return;
	}

	StageIDLeft  = AllStages[0].StageID;
	StageIDRight = AllStages[1].StageID;

	SetStageCardDisplay(Img_Card_Left,  Txt_Stage_Left,  AllStages[0]);
	SetStageCardDisplay(Img_Card_Right, Txt_Stage_Right, AllStages[1]);
}

void URSStageSelectWidget::SetStageCardDisplay(UImage* Img, UTextBlock* Txt, const FStageStaticData& Data)
{
	if (Txt)
	{
		Txt->SetText(Data.DisplayName);
	}

	if (Img && !Data.Thumbnail.IsNull())
	{
		UTexture2D* Tex = Data.Thumbnail.LoadSynchronous();
		if (Tex)
		{
			Img->SetBrushFromTexture(Tex, true);
		}
	}
}

// ── 버튼 핸들러 ───────────────────────────────────────────────────────────────

void URSStageSelectWidget::OnCardLeftClicked()
{
	if (StageIDLeft.IsNone())
	{
		KHS_WARN("RSStageSelectWidget::OnCardLeftClicked — StageIDLeft 미초기화");
		return;
	}

	OnStageSelectedDel.Broadcast(StageIDLeft);
}

void URSStageSelectWidget::OnCardRightClicked()
{
	if (StageIDRight.IsNone())
	{
		KHS_WARN("RSStageSelectWidget::OnCardRightClicked — StageIDRight 미초기화");
		return;
	}

	OnStageSelectedDel.Broadcast(StageIDRight);
}

void URSStageSelectWidget::OnBackClicked()
{
	GET_GI_SUBSYSTEM(UUIManagerSubsystem, UMS)
	UMS->BackPage();
}
