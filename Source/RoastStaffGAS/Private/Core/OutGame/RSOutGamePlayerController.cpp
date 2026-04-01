// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/OutGame/RSOutGamePlayerController.h"
#include "RoastStaffGAS.h"
#include "Subsystems/UIManagerSubsystem.h"
#include "Core/RSGameInstance.h"
#include "Data/EnumUITypes.h"

void ARSOutGamePlayerController::BeginPlay()
{
	Super::BeginPlay();

	SetShowMouseCursor(true);

	FInputModeUIOnly UIOnlyMode;
	SetInputMode(UIOnlyMode);

	OpenFirstWidget();
}

void ARSOutGamePlayerController::OpenFirstWidget()
{
	GET_GI_SUBSYSTEM(UUIManagerSubsystem, UMS);

	// PERSISTENT HUD(아웃게임 공통 레이아웃) 오픈
	UMS->OpenUIByID(EUIID::OUTGAME);

	// 이전 레벨에서 남은 PAGE 이동 이력 제거
	UMS->ClearUIHistory();

	// 첫 PAGE인 로비 화면 오픈
	URSBaseWidget* LobbyWidget = UMS->OpenUIByID(EUIID::LOBBY);
	if (!LobbyWidget)
	{
		KHS_WARN(TEXT("LobbyWidget OPEN FAILED"));
	}

	// RSLobbyWidget이 UIManager를 직접 호출해 PAGE 전환을 처리하므로
	// OGPC 측 추가 델리게이트 바인딩 불필요
}

void ARSOutGamePlayerController::OnCharacterSelectClicked()
{
	GET_GI_SUBSYSTEM(UUIManagerSubsystem, UMS);

	// 현재 로비 PAGE를 히스토리에 쌓고 캐릭터 선택 PAGE로 전환
	UMS->SwitchPageUI(EUIID::CHAR_SELECT);

	// TODO(PLAN_GameFlow_Levels MODULE-5): RSCharacterSelectWidget 구현 완료 후 델리게이트 바인딩
	// Cast<URSCharacterSelectWidget>(UMS->OpenUIByID(EUIID::CHAR_SELECT))->OnCharacterSelectedDel.AddUniqueDynamic(this, &ARSOutGamePlayerController::OnCharacterSelected);
}

void ARSOutGamePlayerController::OnStageSelectClicked()
{
	GET_GI_SUBSYSTEM(UUIManagerSubsystem, UMS);

	// 현재 로비 PAGE를 히스토리에 쌓고 스테이지 선택 PAGE로 전환
	UMS->SwitchPageUI(EUIID::STAGE_SELECT);

	// TODO(PLAN_GameFlow_Levels MODULE-5): RSStageSelectWidget 구현 완료 후 델리게이트 바인딩
	// Cast<URSStageSelectWidget>(UMS->OpenUIByID(EUIID::STAGE_SELECT))->OnStageSelectedDel.AddUniqueDynamic(this, &ARSOutGamePlayerController::OnStageSelected);
}

void ARSOutGamePlayerController::OnSettingClicked()
{
	GET_GI_SUBSYSTEM(UUIManagerSubsystem, UMS);

	UMS->OpenUIByID(EUIID::SETTING);
}

void ARSOutGamePlayerController::OnCharacterSelected(FName CharID)
{
	// TODO(PLAN_GameFlow_Data MODULE-2): RuntimeDataSubsystem::SetSelectedCharacter(CharID)

	GET_GI_SUBSYSTEM(UUIManagerSubsystem, UMS);

	// 선택 완료 후 이전 PAGE(로비)로 복귀
	UMS->BackPage();
}

void ARSOutGamePlayerController::OnStageSelected(FName StageID)
{
	GET_GI(_GI);
	URSGameInstance* GI = Cast<URSGameInstance>(_GI);
	check(GI);

	// 선택한 스테이지 ID 저장 후 TRANSITION을 경유해 STAGE 레벨로 전환
	GI->OpenNextStage(StageID);
}
