// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/RSGameMode.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
#include "RoastStaffGAS.h"
#include "System/EnemySpawner.h"
#include "Character/Player/RSPlayerController.h"
#include "UI/FloatingDamageWidget.h"
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
#include "Character/Player/RSPlayerCharacter.h"
#include "Character/Enemy/EnemyBaseCharacter.h"
#include "Objects/Projectile/EnemyProjectile.h"
#include "Objects/Projectile/BaseProjectile.h"
#include "Core/RSGameInstance.h"
#include "Data/DataTableStructs.h"
#include "Data/RuntimeDataStructs.h"
#include "Subsystems/RuntimeDataSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/LevelStreaming.h"

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

	// LoadingWidget 재생성 — 레벨 전환 시 이전 World의 Widget이 파괴되므로 새 World에서 재생성 필요
	GET_GI_SUBSYSTEM_FROM(UUIManagerSubsystem, UMS, GetGameInstance())

	// 이전 World의 dangling Widget 상태 초기화 (bIsOpen=true 남아있을 수 있음)
	if (URSLoadingWidget* OldWidget = Cast<URSLoadingWidget>(UMS->GetWidgetByID(EUIID::LOADING)))
	{
		if (OldWidget->IsOpen() && !OldWidget->IsInViewport())
		{
			UMS->CloseUIByID(EUIID::LOADING);
		}
	}

	if (URSLoadingWidget* LoadingWidget = Cast<URSLoadingWidget>(UMS->OpenUIByID(EUIID::LOADING)))
	{
		LoadingWidget->SetLoadingProgress(0.9f);  // TransitionGameMode 스트리밍 완료 (0.9) 표시
	}

	GET_GI_SUBSYSTEM(USaveGameSubsystem, SGS)

#if WITH_EDITOR
	if (SGS->GetLastSelectedCharacter().IsNone())
	{
		KHS_WARN("RSGameMode — 에디터 직접 실행: CharID 없음. 디버그 기본값 'Sorceress' 사용");
		SGS->SetLastSelectedCharacter(FName("CHAR_PAINTER"));
	}
#endif

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

	if (bWaitingForLevelLoad && AreAllStreamingLevelsLoaded())
	{
		bWaitingForLevelLoad = false;
		StartStageFlow();
		KHS_INFO(TEXT("스트리밍 레벨 로드 완료 — 스테이지 시작"));
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
	ApplyCharacterMesh(CharID);
	InitDefaultWeapon(CharID);
}

bool ARSGameMode::InitializeStage()
{
	URSGameInstance* GI = Cast<URSGameInstance>(GetGameInstance());
	check(GI);

	CurrentStageID = GI->GetNextStageID();

#if WITH_EDITOR
	if (CurrentStageID.IsNone())
	{
		KHS_WARN("에디터 직접 실행: StageID 없음. 디버그 기본값 'STG_004' 사용");
		GI->SetNextStageID(FName("STG_004"));
		CurrentStageID = FName("STG_004");
	}
#endif

	if (CurrentStageID.IsNone())
	{
		KHS_WARN("NextStageID가 NAME_None. 스테이지 시작 불가.");
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
	TRACE_BOOKMARK(TEXT("GameMode_CollectEnemyClasses_SyncLoad"));
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

	// 데미지 플로팅 위젯 — PlayerController의 클래스를 읽어 단일 출처 유지
	if (ARSPlayerController* PC = Cast<ARSPlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		if (TSubclassOf<UFloatingDamageWidget> WidgetClass = PC->GetFloatingDamageWidgetClass())
		{
			if (DamageFloatingWidgetPoolCount > 0)
			{
				PreWarmList.Add(MakeWidgetRequest(WidgetClass, DamageFloatingWidgetPoolCount));
			}
		}
	}

	// 캐릭터 스킬 이펙트 액터 — RDS에서 선택된 캐릭터 ID 읽어 DT 조회
	GET_GI_SUBSYSTEM_FROM(URuntimeDataSubsystem, RDS, GetGameInstance())
	GET_GI_SUBSYSTEM_FROM(UGameDataSubsystem, GDS, GetGameInstance())
	const FName CharID = RDS->GetSelectedCharacterID();
	if (!CharID.IsNone())
	{
		for (const FCharacterSkillStaticData& SkillData : GDS->GetSkillsByCharacter(CharID))
		{
			// EffectActorClass (PullVortexActor, GroundEffectActor 등)
			TRACE_BOOKMARK(TEXT("GameMode_BuildPreWarmList_SyncLoad"));
			if (!SkillData.EffectActorClass.IsNull())
			{
				if (TSubclassOf<AActor> EffectClass = SkillData.EffectActorClass.LoadSynchronous())
				{
					PreWarmList.Add(MakeActorRequest(EffectClass, SkillEffectActorPoolCount));
				}
			}

			// ProjectileClass (ProjectileSpawn 타입)
			// GetSkillResourceData는 private — GetCharacterSkillExecData 통해 ProjectileClass 조회
			FCharacterSkillExecData ExecData;
			if (GDS->GetCharacterSkillExecData(CharID, SkillData.SkillSlot, ExecData)
				&& !ExecData.ProjectileClass.IsNull())
			{
				if (TSubclassOf<AActor> ProjClass = ExecData.ProjectileClass.LoadSynchronous())
				{
					PreWarmList.Add(MakeActorRequest(ProjClass, SkillProjectilePoolCount));
				}
			}
		}
	}
	else
	{
		KHS_WARN(TEXT("BuildPreWarmList — SelectedCharacterID 없음, 스킬 이펙트 액터 PreWarm 생략"));
	}

	KHS_INFO(TEXT("BuildPreWarmList — 요청 %d건 (에너미 %d클래스 + 투사체 + 위젯 + 스킬이펙트)"), PreWarmList.Num(), UniqueEnemyClasses.Num());

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

	if (AreAllStreamingLevelsLoaded())
	{
		StartStageFlow();
		KHS_INFO(TEXT("프리웜 완료 — 스테이지 시작"));
	}
	else
	{
		bWaitingForLevelLoad = true;
		KHS_INFO(TEXT("프리웜 완료 — 스트리밍 레벨 로드 대기 중..."));
	}
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

bool ARSGameMode::AreAllStreamingLevelsLoaded() const
{
	for (ULevelStreaming* StreamingLevel : GetWorld()->GetStreamingLevels())
	{
		if (!StreamingLevel)
		{
			continue;
		}
		// 로드+표시 요청된 레벨만 검사 — 요청되지 않은 레벨은 스킵
		if (!StreamingLevel->ShouldBeLoaded() || !StreamingLevel->ShouldBeVisible())
		{
			continue;
		}
		if (!StreamingLevel->HasLoadedLevel() || !StreamingLevel->IsLevelVisible())
		{
			return false;
		}
	}
	return true;
}

void ARSGameMode::StartStageFlow()
{
	TRACE_BOOKMARK(TEXT("Stage_Start"));
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

void ARSGameMode::ApplyCharacterMesh(FName CharID)
{
	if (CharID.IsNone())
	{
		KHS_WARN(TEXT("CharID가 NAME_None. 메시 적용 건너뜀."));
		return;
	}

	GET_GI_SUBSYSTEM(UGameDataSubsystem, GDS)

	FCharacterStaticData CharData;
	if (!GDS->GetCharacterStaticData(CharID, CharData))
	{
		KHS_WARN(TEXT("CharID [%s] 데이터 조회 실패. 메시 적용 건너뜀."), *CharID.ToString());
		return;
	}

	if (CharData.Mesh.IsNull())
	{
		KHS_WARN(TEXT("CharID [%s] Mesh가 null. 메시 적용 건너뜀."), *CharID.ToString());
		return;
	}

	if (CharData.AnimBP.IsNull())
	{
		KHS_WARN(TEXT("CharID [%s] AnimBP가 null. 메시 적용 건너뜀."), *CharID.ToString());
		return;
	}

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC)
	{
		KHS_WARN(TEXT("PlayerController를 찾지 못했습니다. 메시 적용 건너뜀."));
		return;
	}

	ARSPlayerCharacter* PlayerChar = PC->GetPawn<ARSPlayerCharacter>();
	if (!PlayerChar)
	{
		KHS_WARN(TEXT("PlayerCharacter를 찾지 못했습니다. 메시 적용 건너뜀."));
		return;
	}

	USkeletalMeshComponent* MeshComp = PlayerChar->GetMesh();
	if (!MeshComp)
	{
		KHS_WARN(TEXT("SkeletalMeshComponent가 null. 메시 적용 건너뜀."));
		return;
	}

	USkeletalMesh* LoadedMesh = CharData.Mesh.LoadSynchronous();
	if (!LoadedMesh)
	{
		KHS_WARN(TEXT("CharID [%s] Mesh 동기 로드 실패."), *CharID.ToString());
		return;
	}

	TSubclassOf<UAnimInstance> LoadedAnimBP = CharData.AnimBP.LoadSynchronous();
	if (!LoadedAnimBP)
	{
		KHS_WARN(TEXT("CharID [%s] AnimBP 동기 로드 실패."), *CharID.ToString());
		return;
	}

	MeshComp->SetSkeletalMesh(LoadedMesh);
	MeshComp->SetAnimInstanceClass(LoadedAnimBP);

	KHS_INFO(TEXT("캐릭터 메시 적용 완료 — CharID: %s"), *CharID.ToString());
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

	// 클리어 시 UnlockStageID 매핑 캐릭터 해금 — UpdateStageRecord(SaveGame 포함) 전에 처리
	if (ResultData.bCleared)
	{
		GET_GI_SUBSYSTEM(UGameDataSubsystem, GDS)
		TArray<FCharacterStaticData> AllChars;
		if (!GDS->GetAllCharacterStaticData(AllChars))
		{
			KHS_WARN(TEXT("GDS 캐릭터 리스트 조회 실패"));
			return;
		}
		
		for (const FCharacterStaticData& CharData : AllChars)
		{
			if (CharData.UnlockType == ECharacterUnlockType::STAGE_CLEAR &&	CharData.UnlockStageID == CurrentStageID)
			{
				SGS->UnlockCharacter(CharData.CharacterID);
				KHS_INFO(TEXT("캐릭터 해금 — CharID: %s (StageID: %s)"),	*CharData.CharacterID.ToString(), *CurrentStageID.ToString());
			}
		}
	}

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
	TRACE_BOOKMARK(TEXT("LevelTransition_Start"));
	GI->OpenNextLevelByName(ELevelName::OUTGAME);
}
