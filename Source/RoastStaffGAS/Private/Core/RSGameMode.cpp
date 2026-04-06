// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/RSGameMode.h"
#include "RoastStaffGAS.h"
#include "System/EnemySpawner.h"
#include "Subsystems/StageManagerSubsystem.h"
#include "Subsystems/SaveGameSubsystem.h"
#include "Subsystems/GameDataSubsystem.h"
#include "Subsystems/EquipmentSubsystem.h"
#include "Subsystems/UIManagerSubsystem.h"
#include "UI/InGame/RSStageResultWidget.h"
#include "Data/EnumUITypes.h"
#include "Character/Player/RSPlayerState.h"
#include "Core/RSGameInstance.h"
#include "Data/DataTableStructs.h"
#include "Data/RuntimeDataStructs.h"
#include "Kismet/GameplayStatics.h"

ARSGameMode::ARSGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ARSGameMode::BeginPlay()
{
	Super::BeginPlay();

	GET_GI_SUBSYSTEM(USaveGameSubsystem, SGS);
	const FName CharID = SGS->GetLastSelectedCharacter();

	InitializePlayer(CharID);

	if (InitializeStage())
	{
		StartStageFlow();
	}
}

void ARSGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsStageEnded)
	{
		CheckStageClearCondition();
	}
}

void ARSGameMode::InitializePlayer(FName CharID)
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	check(PC);

	ARSPlayerState* PS = PC->GetPlayerState<ARSPlayerState>();
	if (!ensureMsgf(PS, TEXT("PlayerState 조회 실패. 캐릭터 스탯 미적용.")))
	{
		check(false);
	}

	PS->ApplyCharacterStats(CharID);
	InitDefaultWeapon(CharID);
}

bool ARSGameMode::InitializeStage()
{
	URSGameInstance* GI = Cast<URSGameInstance>(GetGameInstance());
	check(GI);

	CurrentStageID = GI->GetNextStageID();
	if (CurrentStageID.IsNone())
	{
		KHS_WARN(TEXT("NextStageID가 NAME_None. 스테이지 시작 불가."));
		return false;
	}

	StageStartTime = GetWorld()->GetTimeSeconds();
	return true;
}

void ARSGameMode::StartStageFlow()
{
	//에너미 스포너 가동
	AEnemySpawner* Spawner = Cast<AEnemySpawner>(
		UGameplayStatics::GetActorOfClass(GetWorld(), AEnemySpawner::StaticClass()));

	if (!Spawner)
	{
		KHS_WARN(TEXT("EnemySpawner를 찾지 못했습니다. 스테이지 시작 불가."));
		return;
	}

	GET_WORLD_SUBSYSTEM(UStageManagerSubsystem, StageMgr);
	StageMgr->SetSpawner(Spawner);
	StageMgr->StartStage(CurrentStageID);

	KHS_INFO(TEXT("스테이지 시작 — StageID: %s"), *CurrentStageID.ToString());
}

void ARSGameMode::InitDefaultWeapon(FName CharID)
{
	if (CharID.IsNone())
	{
		KHS_WARN(TEXT("CharID가 NAME_None. 무기 장착 건너뜀."));
		return;
	}

	GET_GI_SUBSYSTEM(UGameDataSubsystem, GDS);

	FCharacterStaticData CharData;
	if (!GDS->GetCharacterStaticData(CharID, CharData))
	{
		KHS_WARN(TEXT("CharID [%s] 데이터 조회 실패. 무기 장착 건너뜀."), *CharID.ToString());
		return;
	}

	if (CharData.DefaultWeaponID.IsNone())
	{
		KHS_WARN(TEXT(" DefaultWeaponID가 NAME_None. 무기 장착 건너뜀."));
		return;
	}

	GET_GI_SUBSYSTEM(UEquipmentSubsystem, EquipSys);
	EquipSys->EquipWeapon(CharData.DefaultWeaponID);

	KHS_INFO(TEXT("DefaultWeapon 장착 완료 — CharID: %s / WeaponID: %s"),
		*CharID.ToString(), *CharData.DefaultWeaponID.ToString());
}

void ARSGameMode::CheckStageClearCondition()
{
	if (CurrentStageID.IsNone())
	{
		return;
	}

	// GDS에서 스테이지 데이터 조회 (TimeLimit)
	GET_GI_SUBSYSTEM(UGameDataSubsystem, GDS);
	FStageStaticData StageData;
	if (!GDS->GetStageData(CurrentStageID, StageData))
	{
		KHS_WARN(TEXT("StageID [%s] 데이터 조회 실패. 클리어 판정 불가."), *CurrentStageID.ToString());
		return;
	}

	// TimeLimit이 0 이하면 클리어 판정 비활성화 (무한 모드)
	if (StageData.TimeLimit <= 0.f)
	{
		return;
	}

	// 경과 시간 계산
	const float ElapsedTime = GetWorld()->GetTimeSeconds() - StageStartTime;

	// TimeLimit 초과 시 클리어
	if (ElapsedTime >= StageData.TimeLimit)
	{
		OnStageCleared();
	}
}

void ARSGameMode::OnStageCleared()
{
	EndStage(true);
}

void ARSGameMode::OnStageFailed()
{
	EndStage(false);
}

void ARSGameMode::EndStage(bool bCleared)
{
	if (bIsStageEnded)
	{
		return;
	}
	bIsStageEnded = true;

	StopStageActivities();
	const FStageResultData ResultData = BuildResultData(bCleared);
	SaveResult(ResultData);
	ShowResultUI(ResultData, bCleared);
}

void ARSGameMode::StopStageActivities()
{
	GET_GI_SUBSYSTEM(UEquipmentSubsystem, EquipSys);
	EquipSys->StopAllFire();

	UGameplayStatics::SetGamePaused(GetWorld(), true);
}

FStageResultData ARSGameMode::BuildResultData(bool bCleared)
{
	const float ElapsedTime = GetWorld()->GetTimeSeconds() - StageStartTime;

	GET_WORLD_SUBSYSTEM(UStageManagerSubsystem, StageMgr);
	const int32 KillCount = StageMgr->GetKillCount();

	KHS_INFO(TEXT("스테이지 %s — StageID: %s | 생존: %.1fs | 처치: %d"), bCleared ? TEXT("클리어!") : TEXT("실패"),
		*CurrentStageID.ToString(), ElapsedTime, KillCount);

	FStageResultData ResultData;
	ResultData.SurvivalTime = ElapsedTime;
	ResultData.KillCount    = KillCount;
	ResultData.bCleared     = bCleared;
	return ResultData;
}

void ARSGameMode::SaveResult(const FStageResultData& ResultData)
{
	GET_GI_SUBSYSTEM(USaveGameSubsystem, SGS);
	SGS->UpdateStageRecord(CurrentStageID, ResultData);
}

void ARSGameMode::ShowResultUI(const FStageResultData& ResultData, bool bCleared)
{
	GET_GI_SUBSYSTEM_FROM(UUIManagerSubsystem, UMS, GetGameInstance());
	URSStageResultWidget* ResultWidget = Cast<URSStageResultWidget>(UMS->OpenUIByID(EUIID::STAGE_RESULT));
	if (!ensureMsgf(ResultWidget, TEXT("StageResultWidget 오픈 실패 — UIManagerSettings STAGE_RESULT 매핑 확인 필요")))
	{
		OnResultConfirmed();
		return;
	}

	GET_GI_SUBSYSTEM(USaveGameSubsystem, SGS);
	const FStageRecord Record = SGS->GetStageRecord(CurrentStageID);
	ResultWidget->SetResultData(
		bCleared,
		ResultData.SurvivalTime,
		ResultData.KillCount,
		Record.BestSurvivalTime,
		Record.BestKillCount,
		CurrentStageID.ToString()
	);
	
	ResultWidget->OnConfirmClickedDel.AddDynamic(this, &ARSGameMode::OnResultConfirmed);
}

void ARSGameMode::OnResultConfirmed()
{
	UGameplayStatics::SetGamePaused(GetWorld(), false);

	URSGameInstance* GI = Cast<URSGameInstance>(GetGameInstance());
	check(GI);

	KHS_INFO(TEXT("OUTGAME 레벨로 복귀 시작..."));
	GI->OpenNextLevelByName(ELevelName::OUTGAME);
}
