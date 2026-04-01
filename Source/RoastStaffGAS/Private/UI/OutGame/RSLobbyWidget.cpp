// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/OutGame/RSLobbyWidget.h"
#include "Subsystems/UIManagerSubsystem.h"
#include "Data/EnumUITypes.h"
#include "RoastStaffGAS.h"
#include "Components/Button.h"

void URSLobbyWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (Btn_CharacterSelect)
	{
		Btn_CharacterSelect->OnClicked.AddDynamic(this, &URSLobbyWidget::OnCharacterSelectClicked);
	}

	if (Btn_StageSelect)
	{
		Btn_StageSelect->OnClicked.AddDynamic(this, &URSLobbyWidget::OnStageSelectClicked);
	}

	if (Btn_Settings)
	{
		Btn_Settings->OnClicked.AddDynamic(this, &URSLobbyWidget::OnSettingsClicked);
	}
}

void URSLobbyWidget::OnCharacterSelectClicked()
{
	GET_GI_SUBSYSTEM_FROM(UUIManagerSubsystem, UMS, GetWorld()->GetGameInstance());

	UMS->SwitchPageUI(EUIID::CHAR_SELECT);
}

void URSLobbyWidget::OnStageSelectClicked()
{
	GET_GI_SUBSYSTEM_FROM(UUIManagerSubsystem, UMS, GetWorld()->GetGameInstance());

	UMS->SwitchPageUI(EUIID::STAGE_SELECT);
}

void URSLobbyWidget::OnSettingsClicked()
{
	GET_GI_SUBSYSTEM_FROM(UUIManagerSubsystem, UMS, GetWorld()->GetGameInstance());

	UMS->OpenUIByID(EUIID::SETTING);
}
