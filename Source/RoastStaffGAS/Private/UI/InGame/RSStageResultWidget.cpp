// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/InGame/RSStageResultWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "System/LoggingSystem.h"

void URSStageResultWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (Btn_Confirm)
	{
		Btn_Confirm->OnClicked.AddDynamic(this, &URSStageResultWidget::OnConfirmButtonClicked);
	}
	else
	{
		KHS_ERROR(TEXT("Btn_Confirm BindWidget FAILED"));
	}
}

void URSStageResultWidget::RefreshUI()
{
	Super::RefreshUI();

	if (Txt_StageTitle)
	{
		Txt_StageTitle->SetText(FText::FromString(StageDisplayName));
	}

	if (Txt_ClearStatus)
	{
		FText StatusText = bIsCleared ? FText::FromString(TEXT("CLEAR!")) : FText::FromString(TEXT("FAILED"));
		Txt_ClearStatus->SetText(StatusText);
	}

	if (Txt_PlayTime)
	{
		int32 Minutes = FMath::FloorToInt(PlayTimeSeconds / 60.f);
		int32 Seconds = FMath::FloorToInt(PlayTimeSeconds) % 60;
		FString TimeString = FString::Printf(TEXT("%d:%02d"), Minutes, Seconds);
		Txt_PlayTime->SetText(FText::FromString(TimeString));
	}

	if (Txt_KillCount)
	{
		Txt_KillCount->SetText(FText::AsNumber(TotalKillCount));
	}

	if (Txt_BestTime)
	{
		int32 Minutes = FMath::FloorToInt(BestTimeSeconds / 60.f);
		int32 Seconds = FMath::FloorToInt(BestTimeSeconds) % 60;
		FString BestTimeString = FString::Printf(TEXT("%d:%02d"), Minutes, Seconds);
		Txt_BestTime->SetText(FText::FromString(BestTimeString));
	}

	if (Txt_BestKill)
	{
		Txt_BestKill->SetText(FText::AsNumber(BestKillCount));
	}
}

void URSStageResultWidget::SetResultData(bool bInCleared, float PlayTime, int32 KillCount, float BestTime, int32 BestKill, const FString& StageName)
{
	bIsCleared = bInCleared;
	PlayTimeSeconds = PlayTime;
	TotalKillCount = KillCount;
	BestTimeSeconds = BestTime;
	BestKillCount = BestKill;
	StageDisplayName = StageName;

	RefreshUI();
}

void URSStageResultWidget::OnConfirmButtonClicked()
{
	KHS_INFO(TEXT("Confirm button clicked - Triggering OUTGAME transition"));
	OnConfirmClickedDel.Broadcast();
}
