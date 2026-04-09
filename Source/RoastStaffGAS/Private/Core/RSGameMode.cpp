// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/RSGameMode.h"
#include "RoastStaffGAS.h"
#include "System/EnemySpawner.h"
#include "Subsystems/PoolingSubsystem.h"
#include "Subsystems/StageManagerSubsystem.h"
#include "Subsystems/SaveGameSubsystem.h"
#include "Subsystems/GameDataSubsystem.h"
#include "Subsystems/EquipmentSubsystem.h"
#include "Subsystems/UIManagerSubsystem.h"
#include "UI/InGame/RSStageResultWidget.h"
#include "UI/Transition/RSLoadingWidget.h"
#include "Data/EnumUITypes.h"
#include "Character/Player/RSPlayerState.h"
#include "Character/Enemy/EnemyBaseCharacter.h"
#include "Objects/Projectile/EnemyProjectile.h"
#include "Core/RSGameInstance.h"
#include "Data/DataTableStructs.h"
#include "Data/RuntimeDataStructs.h"
#include "Kismet/GameplayStatics.h"

FPoolPreWarmRequest ARSGameMode::MakeActorRequest(TSubclassOf<AActor> Class, int32 Count)
{
	FPoolPreWarmRequest Req;
	Req.ActorClass = Class;
	Req.Count = Count;
	return Req;
}

FPoolPreWarmRequest ARSGameMode::MakeWidgetRequest(TSubclassOf<UUserWidget> Class, int32 Count)
{
	FPoolPreWarmRequest Req;
	Req.WidgetClass = Class;
	Req.Count = Count;
	return Req;
}

ARSGameMode::ARSGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ARSGameMode::BeginPlay()
{
	Super::BeginPlay();

	GET_GI_SUBSYSTEM(USaveGameSubsystem, SGS)
	const FName CharID = SGS->GetLastSelectedCharacter();

	InitializePlayer(CharID);

	if (!InitializeStage())
	{
		return;
	}

	CachedSpawner = Cast<AEnemySpawner>(UGameplayStatics::GetActorOfClass(GetWorld(), AEnemySpawner::StaticClass()));

	if (!CachedSpawner)
	{
		KHS_WARN(TEXT("EnemySpawner를 찾지 못했습니다. 스테이지 시작 불가."));
		return;
	}

	InitializePreWarm(CachedSpawner);
}

void ARSGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsPreWarmActive)
	{
		UpdatePreWarmProgress();
	}

	if (!bIsStageEnded)
	{
		CheckStageClearCondition();
	}
}

void ARSGameMode::InitializePlayer(FName CharID)
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	check(PC)

	ARSPlayerState* PS = PC->GetPlayerState<ARSPlayerState>();
	if (!ensureMsgf(PS, TEXT("PlayerState 조회 실패. 캐릭터 스탯 미적용.")))
	{
		check(false)
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

void ARSGameMode::InitializePreWarm(AEnemySpawner* Spawner)
{
	TArray<FPoolPreWarmRequest> PreWarmList = BuildPreWarmList(Spawner);

	if (PreWarmList.IsEmpty())
	{
		KHS_WARN(TEXT("PreWarmList가 비어있음. 즉시 스테이지 시작."));
		CloseLoadingUI();
		StartStageFlow();
		return;
	}

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		PC->DisableInput(PC);
	}

	bIsPreWarmActive = true;

	GET_WORLD_SUBSYSTEM(UPoolingSubsystem, PoolSys)
	PoolSys->OnPreWarmComplete.AddUObject(this, &ARSGameMode::OnPreWarmCompleted);
	PoolSys->RequestAsyncPreWarm(MoveTemp(PreWarmList));

	KHS_INFO(TEXT("프리웜 시작 — StageID: %s"), *CurrentStageID.ToString());
}

TSet<TSubclassOf<AActor>> ARSGameMode::CollectUniqueEnemyClasses() const
{
	GET_GI_SUBSYSTEM_FROM(UGameDataSubsystem, GDS, GetGameInstance())
	
	TSet<TSubclassOf<AActor>> UniqueClasses;
	for (const FWaveStaticData& Wave : GDS->GetWaveDataByStage(CurrentStageID))
	{
		for (const FName& EnemyID : Wave.SpawnEnemyIDs)
		{
			FEnemyStaticData EnemyData;
			if (!GDS->GetEnemyData(EnemyID, EnemyData) || EnemyData.EnemyClass.IsNull())
			{
				continue;
			}
			if (TSubclassOf<AEnemyBaseCharacter> Loaded = EnemyData.EnemyClass.LoadSynchronous())
			{
				UniqueClasses.Add(Loaded);
			}
		}
	}
	return UniqueClasses;
}

TArray<FPoolPreWarmRequest> ARSGameMode::BuildPreWarmList(AEnemySpawner* Spawner)
{
	TArray<FPoolPreWarmRequest> PreWarmList;

	const TSet<TSubclassOf<AActor>> UniqueEnemyClasses = CollectUniqueEnemyClasses();

	// 에너미 클래스 — 클래스당 풀 수량
	const int32 PerClassCount = Spawner->GetPoolCountPerClass();
	for (TSubclassOf<AActor> EnemyClass : UniqueEnemyClasses)
	{
		PreWarmList.Add(MakeActorRequest(EnemyClass, PerClassCount));
	}

	// 에너미 투사체
	if (TSubclassOf<AEnemyProjectile> ProjClass = Spawner->GetEnemyProjectileClass())
	{
		PreWarmList.Add(MakeActorRequest(ProjClass, Spawner->GetProjectilePoolCount()));
	}

	// 데미지 플로팅 위젯
	if (DamageFloatingWidgetClass && DamageFloatingWidgetPoolCount > 0)
	{
		PreWarmList.Add(MakeWidgetRequest(DamageFloatingWidgetClass, DamageFloatingWidgetPoolCount));
	}

	KHS_INFO(TEXT("BuildPreWarmList — 요청 %d건 (에너미 %d클래스 + 투사체 + 위젯)"),
		PreWarmList.Num(), UniqueEnemyClasses.Num());
	return PreWarmList;
}

void ARSGameMode::OnPreWarmCompleted()
{
	bIsPreWarmActive = false;

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		PC->EnableInput(PC);
	}

	CloseLoadingUI();
	StartStageFlow();

	KHS_INFO(TEXT("프리웜 완료 — 스테이지 시작"));
}

void ARSGameMode::CloseLoadingUI()
{
	if (URSLoadingWidget* LoadingWidget = GetLoadingWidget())
	{
		LoadingWidget->FinishLoading();
	}

	GET_GI_SUBSYSTEM_FROM(UUIManagerSubsystem, UMS, GetGameInstance())
	UMS->CloseUIByID(EUIID::LOADING);
}

URSLoadingWidget* ARSGameMode::GetLoadingWidget() const
{
	GET_GI_SUBSYSTEM_FROM(UUIManagerSubsystem, UMS, GetGameInstance())
	return Cast<URSLoadingWidget>(UMS->GetWidgetByID(EUIID::LOADING));
}

void ARSGameMode::UpdatePreWarmProgress()
{
	GET_WORLD_SUBSYSTEM(UPoolingSubsystem, PoolSys)
	if (URSLoadingWidget* LoadingWidget = GetLoadingWidget())
	{
		LoadingWidget->SetLoadingProgress(PoolSys->GetPreWarmProgress());
	}
}

void ARSGameMode::StartStageFlow()
{
	if (!CachedSpawner)
	{
		KHS_WARN(TEXT("CachedSpawner가 없습니다. 스테이지 시작 불가."));
		return;
	}

	GET_WORLD_SUBSYSTEM(UStageManagerSubsystem, StageMgr)
	StageMgr->SetSpawner(CachedSpawner);
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

	GET_GI_SUBSYSTEM(UGameDataSubsystem, GDS)

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

	GET_GI_SUBSYSTEM(UEquipmentSubsystem, EquipSys)
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
	GET_GI_SUBSYSTEM(UGameDataSubsystem, GDS)
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
	const float ElapsedTime = (static_cast<float>(GetWorld()->GetTimeSeconds()) - StageStartTime);

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
	GET_GI_SUBSYSTEM(UEquipmentSubsystem, EquipSys)
	EquipSys->StopAllFire();

	UGameplayStatics::SetGamePaused(GetWorld(), true);
}

FStageResultData ARSGameMode::BuildResultData(bool bCleared)
{
	const float ElapsedTime = GetWorld()->GetTimeSeconds() - StageStartTime;

	GET_WORLD_SUBSYSTEM(UStageManagerSubsystem, StageMgr)
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
	GET_GI_SUBSYSTEM(USaveGameSubsystem, SGS)
	SGS->UpdateStageRecord(CurrentStageID, ResultData);
}

void ARSGameMode::ShowResultUI(const FStageResultData& ResultData, bool bCleared)
{
	GET_GI_SUBSYSTEM_FROM(UUIManagerSubsystem, UMS, GetGameInstance())
	URSStageResultWidget* ResultWidget = Cast<URSStageResultWidget>(UMS->OpenUIByID(EUIID::STAGE_RESULT));
	if (!ensureMsgf(ResultWidget, TEXT("StageResultWidget 오픈 실패 — UIManagerSettings STAGE_RESULT 매핑 확인 필요")))
	{
		OnResultConfirmed();
		return;
	}

	GET_GI_SUBSYSTEM(USaveGameSubsystem, SGS)
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
	// 레벨 전환 전 타이머 전량 정리 — SetGamePaused(false) 이전에 실행해야 함
	GET_GI_SUBSYSTEM(UEquipmentSubsystem, EquipSys)
	EquipSys->DeinitializeSubsystem();

	UGameplayStatics::SetGamePaused(GetWorld(), false);

	URSGameInstance* GI = Cast<URSGameInstance>(GetGameInstance());
	check(GI)

	KHS_INFO(TEXT("OUTGAME 레벨로 복귀 시작..."));
	GI->OpenNextLevelByName(ELevelName::OUTGAME);
}
