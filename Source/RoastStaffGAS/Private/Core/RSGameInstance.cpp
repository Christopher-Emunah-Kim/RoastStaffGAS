// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/RSGameInstance.h"
#include "Subsystems/UIManagerSubsystem.h"
#include "System/MapSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "System/LoggingSystem.h"

void URSGameInstance::OpenNextLevelByName(ELevelName Level)
{
	// 목적지를 먼저 저장해 두면 TRANSITION 레벨이 OpenNextLevelLatent()를 호출할 때 사용
	SetNextLevelName(Level);

	// 현재 레벨의 UI 캐시 전체 파괴 — 위젯은 레벨마다 새로 생성
	if (UUIManagerSubsystem* UIManager = GetSubsystem<UUIManagerSubsystem>())
	{
		UIManager->ResetAllUIStates();
	}

	// 동일 프레임에서 OpenLevel을 호출하면 RemoveFromParent 중인 위젯과 충돌할 수 있으므로
	// 한 프레임 뒤에 실행 — 0.1s는 정리 완료를 보장하는 최소 여유값
	FTimerHandle Unused;
	GetWorld()->GetTimerManager().SetTimer(
		Unused,	this,	&URSGameInstance::DoOpenTransitionLevel,
		0.1f,	false);
}

void URSGameInstance::DoOpenTransitionLevel()
{
	// TRANSITION 레벨 이름은 MapSettings 없이 고정 — 인프라 레벨이므로 하드코딩 허용
	UGameplayStatics::OpenLevel(this, TEXT("TRANSITION"));
}

void URSGameInstance::OpenNextLevelLatent()
{
	const UMapSettings* Settings = UMapSettings::Get();
	if (!ensureMsgf(Settings, TEXT("MapSettings를 찾을 수 없음")))
	{
		return;
	}

	const TSoftObjectPtr<UWorld>* LevelAsset = Settings->LevelMap.Find(NextLevelName);
	if (!LevelAsset || LevelAsset->IsNull())
	{
		KHS_ERROR(TEXT("ELevelName(%d)에 매핑된 레벨 없음. MapSettings 확인 필요"), static_cast<uint8>(NextLevelName));
		return;
	}

	// Soft 경로에서 패키지 이름 추출 후 OpenLevel
	const FString LevelPath = LevelAsset->GetLongPackageName();
	UGameplayStatics::OpenLevel(this, *LevelPath);
}

void URSGameInstance::OpenNextStage(FName StageID)
{
	SetNextStageID(StageID);

	// StageManagerSubsystem::LoadStage() 미구현 — DEFERRED
	// TODO: StageManagerSubsystem 준비 후 LoadStage(StageID) 호출 추가

	OpenNextLevelByName(ELevelName::STAGE);
}
