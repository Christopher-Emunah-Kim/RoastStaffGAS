// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/RSGameInstance.h"
#include "Subsystems/UIManagerSubsystem.h"
#include "System/MapSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "System/LoggingSystem.h"
#include "Misc/PackageName.h"

void URSGameInstance::OpenNextLevelByName(ELevelName Level)
{
	// 목적지를 먼저 저장해 두면 TRANSITION 레벨이 OpenNextLevelLatent()를 호출할 때 사용
	SetNextLevelName(Level);

	// 현재 레벨의 UI 캐시 전체 파괴 — 위젯은 레벨마다 새로 생성
	if (UUIManagerSubsystem* UIManager = GetSubsystem<UUIManagerSubsystem>())
	{
		UIManager->ResetAllUIStates();
	}

	// 동일 프레임에서 OpenLevel을 호출하면 RemoveFromParent 중인 위젯과 충돌할 수 있으므로 한 프레임 뒤에 실행 
	FTimerHandle TempTimer;
	GetWorld()->GetTimerManager().SetTimer(
		TempTimer,	this,	&URSGameInstance::DoOpenTransitionLevel,0.1f,	false);
}

void URSGameInstance::DoOpenTransitionLevel()
{
	const UMapSettings* Settings = UMapSettings::Get();
	if (!ensureMsgf(Settings, TEXT("MapSettings를 찾을 수 없음")))
	{
		return;
	}

	const TSoftObjectPtr<UWorld>* LevelAsset = Settings->GetWorldMapBy(ELevelName::TRANSITION);
	if (!LevelAsset || LevelAsset->IsNull())
	{
		KHS_ERROR(TEXT("ELevelName::TRANSITION에 매핑된 레벨 없음. MapSettings 확인 필요"));
		return;
	}

	const FName LevelName = FName(*FPackageName::GetShortName(LevelAsset->GetLongPackageName()));
	UGameplayStatics::OpenLevel(this, LevelName);
}

void URSGameInstance::OpenNextLevelLatent()
{
	const UMapSettings* Settings = UMapSettings::Get();
	if (!ensureMsgf(Settings, TEXT("MapSettings를 찾을 수 없음")))
	{
		return;
	}

	const TSoftObjectPtr<UWorld>* LevelAsset = Settings->GetWorldMapBy(NextLevelName);
	if (!LevelAsset || LevelAsset->IsNull())
	{
		KHS_ERROR(TEXT("ELevelName(%d)에 매핑된 레벨 없음. MapSettings 확인 필요"), static_cast<uint8>(NextLevelName));
		return;
	}

	// GetLongPackageName() → /Game/Map/Map_Intro 형식 → ShortName → "Map_Intro"
	// OpenLevel URL은 패키지 경로가 아닌 맵 파일 이름을 사용
	const FName LevelName = FName(*FPackageName::GetShortName(LevelAsset->GetLongPackageName()));
	UGameplayStatics::OpenLevel(this, LevelName);
}

void URSGameInstance::OpenNextStage(FName StageID)
{
	SetNextStageID(StageID);

	// StageManagerSubsystem::LoadStage() 미구현 — DEFERRED
	// TODO: StageManagerSubsystem 준비 후 LoadStage(StageID) 호출 추가

	// TODO(PLAN_OutGame_SelectUI MODULE-5): GDS::GetStageStaticData(StageID).WorldLevel로 교체
	// 현재는 모든 스테이지를 STAGE_1 레벨로 임시 연결
	OpenNextLevelByName(ELevelName::STAGE_1);
}
