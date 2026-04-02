// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/Transition/RSTransitionGameMode.h"
#include "RoastStaffGAS.h"
#include "UI/Transition/RSLoadingWidget.h"
#include "Subsystems/UIManagerSubsystem.h"
#include "Subsystems/RuntimeDataSubsystem.h"
#include "Subsystems/GameDataSubsystem.h"
#include "Core/RSGameInstance.h"
#include "System/MapSettings.h"
#include "Data/EnumUITypes.h"
#include "Data/DataTableStructs.h"
#include "Engine/AssetManager.h"
#include "Kismet/GameplayStatics.h"

ARSTransitionGameMode::ARSTransitionGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ARSTransitionGameMode::BeginPlay()
{
	Super::BeginPlay();

	GET_GI_SUBSYSTEM(UUIManagerSubsystem, UMS);
	LoadingWidget = Cast<URSLoadingWidget>(UMS->OpenUIByID(EUIID::LOADING));
	if (LoadingWidget)
	{
		LoadingWidget->SetLoadingProgress(0.f);
	}
	else
	{
		KHS_WARN(TEXT("LoadingWidget 오픈 실패"));
	}

	PreloadAssetsAsync();
}

void ARSTransitionGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsLoadingLevel || !LoadingWidget)
	{
		return;
	}

	// FakeProgress: 0 → 0.9 보간 (실제 완료 전 최대 90%까지만)
	CurrentFakeProgress = FMath::FInterpTo(CurrentFakeProgress, 0.9f, DeltaTime, 1.5f);
	LoadingWidget->SetLoadingProgress(CurrentFakeProgress);
}

void ARSTransitionGameMode::PreloadAssetsAsync()
{
	GET_GI(_GI);
	URSGameInstance* GI = Cast<URSGameInstance>(_GI);
	check(GI);

	URuntimeDataSubsystem* RDS = GI->GetSubsystem<URuntimeDataSubsystem>();
	UGameDataSubsystem*    GDS = GI->GetSubsystem<UGameDataSubsystem>();

	if (!ensureMsgf(RDS && GDS, TEXT("RDS 또는 GDS 없음")))
	{
		StartLevelStreaming();
		return;
	}

	// 스테이지 웨이브에서 등장 에너미 ID 수집
	TArray<FName> EnemyIDList;
	const FName NextStageID = GI->GetNextStageID();
	
	if (!NextStageID.IsNone())
	{
		TArray<FWaveStaticData> Waves = GDS->GetWaveDataByStage(NextStageID);
		for (const FWaveStaticData& Wave : Waves)
		{
			for (const FName& EnemyID : Wave.SpawnEnemyIDs)
			{
				EnemyIDList.AddUnique(EnemyID);
			}
		}
	}

	TArray<FSoftObjectPath> PreloadPaths;
	RDS->GatherPreloadAssets(PreloadPaths, true, EnemyIDList);

	if (PreloadPaths.IsEmpty())
	{
		// 프리로드할 에셋 없음(캐릭터 미선택 등) — 즉시 레벨 스트리밍 진행
		StartLevelStreaming();
		return;
	}

	// 비동기 로드 완료 후 StartLevelStreaming 호출
	FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
	StreamableHandle = Streamable.RequestAsyncLoad(
		PreloadPaths, FStreamableDelegate::CreateUObject(this, &ARSTransitionGameMode::StartLevelStreaming)
	);
}

void ARSTransitionGameMode::StartLevelStreaming()
{
	GET_GI(_GI);
	URSGameInstance* GI = Cast<URSGameInstance>(_GI);
	check(GI);

	const UMapSettings* Settings = UMapSettings::Get();
	if (!ensureMsgf(Settings, TEXT("MapSettings 없음")))
	{
		return;
	}

	const TSoftObjectPtr<UWorld>* LevelAsset = Settings->GetWorldMapBy(GI->GetNextLevelName());
	if (!LevelAsset || LevelAsset->IsNull())
	{
		KHS_ERROR(TEXT("목적지 레벨 매핑 없음. MapSettings 확인 필요"));
		return;
	}

	bIsLoadingLevel = true;

	FLatentActionInfo LatentInfo;
	LatentInfo.CallbackTarget    = this;
	LatentInfo.ExecutionFunction = FName("OnLevelPreloadCompleted");
	LatentInfo.Linkage           = 0;
	LatentInfo.UUID              = FMath::Rand();

	const FName LevelFName = LevelAsset->GetLongPackageFName();
	UGameplayStatics::LoadStreamLevel(this, LevelFName, false, false, LatentInfo);
}

void ARSTransitionGameMode::OnLevelPreloadCompleted()
{
	bIsLoadingLevel = false;

	if (LoadingWidget)
	{
		LoadingWidget->FinishLoading();
	}

	// 완료 연출 후 1초 대기 → 최종 레벨 이동
	FTimerHandle TempTimer;
	GetWorld()->GetTimerManager().SetTimer(
		TempTimer,this,	&ARSTransitionGameMode::OpenNextLevel,1.0f,false);
}

void ARSTransitionGameMode::OpenNextLevel()
{
	GET_GI(_GI);
	URSGameInstance* GI = Cast<URSGameInstance>(_GI);
	check(GI);

	GI->OpenNextLevelLatent();
}
