// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/Intro/RSIntroPlayerController.h"
#include "RoastStaffGAS.h"
#include "Subsystems/UIManagerSubsystem.h"
#include "Core/RSGameInstance.h"
#include "Data/EnumUITypes.h"
#include "UI/Intro/RSIntroWidget.h"
#include "UI/Intro/RSTitleWidget.h"

void ARSIntroPlayerController::BeginPlay()
{
	Super::BeginPlay();

	SetShowMouseCursor(false);

	FInputModeUIOnly UIOnlyMode;
	SetInputMode(UIOnlyMode);

	OpenFirstWidget();
}

void ARSIntroPlayerController::OpenFirstWidget()
{
	GET_GI_SUBSYSTEM(UUIManagerSubsystem, UMS);
	
	UMS->OpenUIByID(EUIID::BACKGROUND);
	URSIntroWidget* IntroWidget = Cast<URSIntroWidget>(UMS->OpenUIByID(EUIID::INTRO));
	if (!IntroWidget)
	{
		KHS_WARN(TEXT("IntroWidget OPEN FAILED"));
	}
	
	IntroWidget->OnTitleOpenRequestedDel.AddUniqueDynamic(this, &ARSIntroPlayerController::OpenTitleScreen);
}

void ARSIntroPlayerController::OpenTitleScreen()
{
	GET_GI_SUBSYSTEM(UUIManagerSubsystem, UMS);
	URSTitleWidget* TitleWidget = Cast<URSTitleWidget>(UMS->OpenUIByID(EUIID::TITLE));
	if (!TitleWidget)
	{
		KHS_WARN(TEXT("TitleWidget OPEN FAILED"));
	}
	
	TitleWidget->OnStartGameRequestedDel.AddUniqueDynamic(this, &ARSIntroPlayerController::OnStartGameClicked);
}

void ARSIntroPlayerController::OnStartGameClicked()
{
	GET_GI(_GI);
	URSGameInstance* GI = Cast<URSGameInstance>(_GI);
	check(GI);

	GI->OpenNextLevelByName(ELevelName::OUTGAME);
}
