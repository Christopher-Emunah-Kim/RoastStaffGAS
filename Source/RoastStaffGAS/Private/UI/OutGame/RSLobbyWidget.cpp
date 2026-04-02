// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/OutGame/RSLobbyWidget.h"
#include "Core/OutGame/RSOutGamePlayerController.h"
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

	if (Btn_Settings)
	{
		Btn_Settings->OnClicked.AddDynamic(this, &URSLobbyWidget::OnSettingsClicked);
	}
}

void URSLobbyWidget::OnCharacterSelectClicked()
{
	// 페이지 전환 + 델리게이트 바인딩은 OGPC가 담당
	ARSOutGamePlayerController* OGPC = Cast<ARSOutGamePlayerController>(GetOwningPlayer());
	if (OGPC)
	{
		OGPC->OnCharacterSelectClicked();
	}
}

void URSLobbyWidget::OnSettingsClicked()
{
	GET_GI_SUBSYSTEM_FROM(UUIManagerSubsystem, UMS, GetWorld()->GetGameInstance());

	UMS->OpenUIByID(EUIID::SETTING);
}
