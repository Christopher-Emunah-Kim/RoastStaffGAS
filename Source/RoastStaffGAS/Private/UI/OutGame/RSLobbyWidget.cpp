// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/OutGame/RSLobbyWidget.h"
#include "UI/OutGame/LobbyCharInfoPanel.h"
#include "Subsystems/UIManagerSubsystem.h"
#include "Data/EnumUITypes.h"
#include "RoastStaffGAS.h"
#include "Components/Button.h"
#include "System/LoggingSystem.h"

void URSLobbyWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	// 패널은 기본 숨김 — 캐릭터 클릭 시 ShowCharInfo()로 표시
	if (Panel_CharInfo)
	{
		Panel_CharInfo->Hide();
	}
}

void URSLobbyWidget::ShowCharInfo(FName CharID)
{
	if (!Panel_CharInfo)
	{
		KHS_WARN("Panel_CharInfo 미바인딩 (WBP 확인)");
		return;
	}

	Panel_CharInfo->Show(CharID);
}

void URSLobbyWidget::HideCharInfo()
{
	if (Panel_CharInfo)
	{
		Panel_CharInfo->Hide();
	}
}
