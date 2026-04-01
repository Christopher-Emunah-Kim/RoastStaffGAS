// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/Transition/RSTransitionGameMode.h"
#include "RoastStaffGAS.h"
#include "UI/Transition/RSLoadingWidget.h"
#include "Subsystems/UIManagerSubsystem.h"
#include "Core/RSGameInstance.h"
#include "System/MapSettings.h"
#include "Data/EnumUITypes.h"
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
		KHS_WARN(TEXT("BeginPlay — LoadingWidget 오픈 실패"));
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
	// TODO(PLAN_Data MODULE-2): RuntimeDS::GatherPreloadAssets(OutPaths) → AssetManager::RequestAsyncLoad 교체 예정
	// 현재는 에셋 수집 없이 바로 레벨 스트리밍 진행
	
	StartLevelStreaming();
}

void ARSTransitionGameMode::StartLevelStreaming()
{
	GET_GI(_GI);
	URSGameInstance* GI = Cast<URSGameInstance>(_GI);
	check(GI);

	const UMapSettings* Settings = UMapSettings::Get();
	if (!ensureMsgf(Settings, TEXT("StartLevelStreaming — MapSettings 없음")))
	{
		return;
	}

	const TSoftObjectPtr<UWorld>* LevelAsset = Settings->GetWorldMapBy(GI->GetNextLevelName());
	if (!LevelAsset || LevelAsset->IsNull())
	{
		KHS_ERROR(TEXT("StartLevelStreaming — 목적지 레벨 매핑 없음. MapSettings 확인 필요"));
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
