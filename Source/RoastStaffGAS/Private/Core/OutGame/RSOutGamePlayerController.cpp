// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/OutGame/RSOutGamePlayerController.h"
#include "RoastStaffGAS.h"
#include "Subsystems/UIManagerSubsystem.h"
#include "Subsystems/SaveGameSubsystem.h"
#include "Subsystems/RuntimeDataSubsystem.h"
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

void ARSOutGamePlayerController::EndPlay(const EEndPlayReason::Type Reason)
{
	// 레벨 종료 전 위젯 델리게이트 해제
	if (CachedCharSelectWidget)
	{
		CachedCharSelectWidget->OnCharacterSelectedDel.RemoveDynamic(this, &ThisClass::OnCharacterSelected);
		CachedCharSelectWidget->OnStageSelectRequestedDel.RemoveDynamic(this, &ThisClass::OnStageSelectClicked);
	}

	if (CachedStageSelectWidget)
	{
		CachedStageSelectWidget->OnStageSelectedDel.RemoveDynamic(this, &ThisClass::OnStageSelected);
	}

	Super::EndPlay(Reason);
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
		KHS_WARN(TEXT("ARSOutGamePlayerController::OpenFirstWidget — LobbyWidget OPEN FAILED"));
	}

}

void ARSOutGamePlayerController::OnCharacterSelectClicked()
{
	GET_GI_SUBSYSTEM(UUIManagerSubsystem, UMS);

	// 페이지 전환 후 반환된 위젯에 델리게이트 바인딩 (AddUniqueDynamic — 중복 방지)
	URSBaseWidget* Raw = UMS->SwitchPageUI(EUIID::CHAR_SELECT);
	CachedCharSelectWidget = Cast<URSCharacterSelectWidget>(Raw);
	if (CachedCharSelectWidget)
	{
		CachedCharSelectWidget->OnCharacterSelectedDel.AddUniqueDynamic(this, &ThisClass::OnCharacterSelected);
		CachedCharSelectWidget->OnStageSelectRequestedDel.AddUniqueDynamic(this, &ThisClass::OnStageSelectClicked);
	}
	else
	{
		KHS_WARN(TEXT("Cast 실패"));
	}
}

void ARSOutGamePlayerController::OnStageSelectClicked()
{
	GET_GI_SUBSYSTEM(UUIManagerSubsystem, UMS);

	// CharacterSelectWidget::OnStageSelectRequestedDel 수신 → 스테이지 선택 PAGE 전환 + 바인딩
	URSBaseWidget* Raw = UMS->SwitchPageUI(EUIID::STAGE_SELECT);
	
	CachedStageSelectWidget = Cast<URSStageSelectWidget>(Raw);
	if (CachedStageSelectWidget)
	{
		CachedStageSelectWidget->OnStageSelectedDel.AddUniqueDynamic(this, &ThisClass::OnStageSelected);
	}
	else
	{
		KHS_WARN(TEXT("ARSOutGamePlayerController::OnStageSelectClicked — Cast 실패"));
	}
}

void ARSOutGamePlayerController::OnSettingClicked()
{
	GET_GI_SUBSYSTEM(UUIManagerSubsystem, UMS);

	UMS->OpenUIByID(EUIID::SETTING);
}

void ARSOutGamePlayerController::OnCharacterSelected(FName CharID)
{
	// 캐릭터 선택 확정 → RDS 메모리 업데이트 (디스크 저장은 스테이지 진입 직전 일괄)
	GET_GI_SUBSYSTEM(URuntimeDataSubsystem, RDS);

	RDS->SetSelectedCharacter(CharID);
}

void ARSOutGamePlayerController::OnStageSelected(FName StageID)
{
	GET_GI_SUBSYSTEM(URuntimeDataSubsystem, RDS);

	// 캐릭터 미선택 방어 — 정상 흐름에서는 CharSelectWidget이 Btn_StageSelect를 disabled로 유지
	if (RDS->GetSelectedCharacterID().IsNone())
	{
		KHS_WARN(TEXT("선택된 캐릭터 없음. 진입 취소."));
		return;
	}

	// 마지막 플레이 스테이지 기록 — StageSelectWidget 복원용
	GET_GI_SUBSYSTEM(USaveGameSubsystem, SGS);
	SGS->SetLastPlayedStageID(StageID);

	// RDS 메모리 → SGS 동기화 후 디스크 저장 (LastPlayedStageID 포함 일괄)
	RDS->SerializeToPersistentData();
	SGS->SaveGame();

	GET_GI(_GI);
	URSGameInstance* GI = Cast<URSGameInstance>(_GI);
	check(GI);

	GI->OpenNextStage(StageID);
}
