// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/OutGame/RSOutGamePlayerController.h"
#include "UI/OutGame/RSLobbyWidget.h"
#include "RoastStaffGAS.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystems/UIManagerSubsystem.h"
#include "Subsystems/SaveGameSubsystem.h"
#include "Subsystems/RuntimeDataSubsystem.h"
#include "Core/RSGameInstance.h"
#include "Data/EnumUITypes.h"
#include "System/LoggingSystem.h"

void ARSOutGamePlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	UpdateLobbyHover();
}

void ARSOutGamePlayerController::BeginPlay()
{
	Super::BeginPlay();

	// 클릭 감지 활성화 (hover는 PlayerTick에서 수동 처리)
	bEnableClickEvents = true;

	SetShowMouseCursor(true);

	FInputModeGameAndUI GameAndUIMode;
	GameAndUIMode.SetHideCursorDuringCapture(false);
	GameAndUIMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
	SetInputMode(GameAndUIMode);

	InitLobbyActors();
	OpenFirstWidget();
	SetViewToOverview();
}

void ARSOutGamePlayerController::EndPlay(const EEndPlayReason::Type Reason)
{
	for (ALobbyCharacterActor* LobbyChar : LobbyCharacters)
	{
		if (LobbyChar)
		{
			LobbyChar->OnCharacterClickedDel.RemoveDynamic(this, &ThisClass::OnLobbyCharacterClicked);
		}
	}

	if (CachedStageSelectWidget)
	{
		CachedStageSelectWidget->OnStageSelectedDel.RemoveDynamic(this, &ThisClass::OnStageSelected);
	}

	Super::EndPlay(Reason);
}

void ARSOutGamePlayerController::OpenFirstWidget()
{
	GET_GI_SUBSYSTEM(UUIManagerSubsystem, UMS)

	UMS->OpenUIByID(EUIID::OUTGAME);
	UMS->ClearUIHistory();

	URSBaseWidget* LobbyWidget = UMS->OpenUIByID(EUIID::LOBBY);
	if (!LobbyWidget)
	{
		KHS_WARN("OpenFirstWidget — LobbyWidget OPEN FAILED");
		return;
	}

	CachedLobbyWidget = Cast<URSLobbyWidget>(LobbyWidget);
	if (!CachedLobbyWidget)
	{
		KHS_WARN("OpenFirstWidget — URSLobbyWidget Cast 실패");
	}
}

void ARSOutGamePlayerController::InitLobbyActors()
{
	// LobbyCharacterActor 탐색 + 클릭 델리게이트 바인딩
	TArray<AActor*> FoundCharacters;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALobbyCharacterActor::StaticClass(), FoundCharacters);

	for (AActor* Actor : FoundCharacters)
	{
		ALobbyCharacterActor* LobbyChar = Cast<ALobbyCharacterActor>(Actor);
		if (LobbyChar)
		{
			LobbyChar->OnCharacterClickedDel.AddUniqueDynamic(this, &ThisClass::OnLobbyCharacterClicked);
			LobbyCharacters.Add(LobbyChar);
		}
	}

	if (LobbyCharacters.IsEmpty())
	{
		KHS_WARN("레벨에 LobbyCharacterActor 없음");
	}

	// 전체 조망 카메라 탐색
	TArray<AActor*> CameraActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("LobbyOverviewCamera"), CameraActors);

	if (!CameraActors.IsEmpty())
	{
		OverviewCameraRef = CameraActors[0];
	}
	else
	{
		KHS_WARN("LobbyOverviewCamera 태그 Actor 없음");
	}
}

void ARSOutGamePlayerController::SetViewToOverview()
{
	if (!OverviewCameraRef)
	{
		KHS_WARN("SetViewToOverview — OverviewCameraRef 없음");
		return;
	}

	SetViewTargetWithBlend(OverviewCameraRef, OverviewBlendTime);
}

void ARSOutGamePlayerController::OnLobbyCharacterClicked(FName CharID)
{
	// 선택 캐릭터 RDS 반영
	GET_GI_SUBSYSTEM(URuntimeDataSubsystem, RDS)
	RDS->SetSelectedCharacter(CharID);

	// 해당 캐릭터 카메라로 블렌드
	for (ALobbyCharacterActor* LobbyChar : LobbyCharacters)
	{
		if (!LobbyChar)
		{
			continue;
		}

		if (LobbyChar->GetCharacterID() == CharID)
		{
			AActor* CharCamera = LobbyChar->GetCharacterCamera();
			if (CharCamera)
			{
				SetViewTargetWithBlend(CharCamera, LobbyChar->GetCameraBlendTime());
			}
			else
			{
				KHS_WARN("CharacterCameraRef 미할당 (BP 에디터 확인 필요)");
			}
			break;
		}
	}

	// 우측 패널 표시 — LobbyWidget 내 BindWidget으로 전체화면 오버레이 없이 토글
	if (CachedLobbyWidget)
	{
		CachedLobbyWidget->ShowCharInfo(CharID);
	}
}

void ARSOutGamePlayerController::OnConfirmClicked()
{
	GET_GI_SUBSYSTEM(URuntimeDataSubsystem, RDS)

	if (RDS->GetSelectedCharacterID().IsNone())
	{
		KHS_WARN("캐릭터 미선택. 스테이지 선택 진입 불가");
		return;
	}

	GET_GI_SUBSYSTEM(UUIManagerSubsystem, UMS)

	URSBaseWidget* Raw = UMS->SwitchPageUI(EUIID::STAGE_SELECT);
	CachedStageSelectWidget = Cast<URSStageSelectWidget>(Raw);
	if (CachedStageSelectWidget)
	{
		CachedStageSelectWidget->OnStageSelectedDel.AddUniqueDynamic(this, &ThisClass::OnStageSelected);
	}
	else
	{
		KHS_WARN("StageSelectWidget Cast 실패");
	}
}

void ARSOutGamePlayerController::OnBackClicked()
{
	if (CachedLobbyWidget)
	{
		CachedLobbyWidget->HideCharInfo();
	}

	SetViewToOverview();
}

void ARSOutGamePlayerController::OnSettingClicked()
{
	GET_GI_SUBSYSTEM(UUIManagerSubsystem, UMS)
	UMS->OpenUIByID(EUIID::SETTING);
}

void ARSOutGamePlayerController::OnStageSelected(FName StageID)
{
	GET_GI_SUBSYSTEM(URuntimeDataSubsystem, RDS)

	// 캐릭터 미선택 방어 — 정상 흐름에서는 OnConfirmClicked 진입 시 이미 검증됨
	if (RDS->GetSelectedCharacterID().IsNone())
	{
		KHS_WARN("선택된 캐릭터 없음. 진입 취소");
		return;
	}

	// 마지막 플레이 스테이지 기록 + 저장
	GET_GI_SUBSYSTEM(USaveGameSubsystem, SGS)
	SGS->SetLastPlayedStageID(StageID);

	RDS->SerializeToPersistentData();
	SGS->SaveGame();

	GET_GI(_GI)
	URSGameInstance* GI = Cast<URSGameInstance>(_GI);
	check(GI);

	GI->OpenNextStage(StageID);
}

void ARSOutGamePlayerController::UpdateLobbyHover()
{
	// 커서 아래 Visibility 채널 트레이스
	FHitResult HitResult;
	const bool bHit = GetHitResultUnderCursor(ECC_Visibility, false, HitResult);

	ALobbyCharacterActor* NewHovered = nullptr;
	if (bHit)
	{
		NewHovered = Cast<ALobbyCharacterActor>(HitResult.GetActor());
	}

	// 이전 프레임과 동일하면 아무것도 안 함
	if (NewHovered == CurrentHoveredLobbyChar)
	{
		return;
	}

	if (CurrentHoveredLobbyChar)
	{
		CurrentHoveredLobbyChar->SetOutlineActive(false);
	}

	CurrentHoveredLobbyChar = NewHovered;

	if (CurrentHoveredLobbyChar)
	{
		CurrentHoveredLobbyChar->SetOutlineActive(true);
	}
}
