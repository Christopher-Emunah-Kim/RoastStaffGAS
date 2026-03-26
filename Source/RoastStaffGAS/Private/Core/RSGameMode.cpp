// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/RSGameMode.h"
#include "RoastStaffGAS.h"
#include "System/EnemySpawner.h"
#include "Subsystems/StageManagerSubsystem.h"
#include "Kismet/GameplayStatics.h"

void ARSGameMode::BeginPlay()
{
	Super::BeginPlay();

	// 맵에 배치된 EnemySpawner 탐색
	AEnemySpawner* Spawner = Cast<AEnemySpawner>(
		UGameplayStatics::GetActorOfClass(GetWorld(), AEnemySpawner::StaticClass()));

	if (!Spawner)
	{
		KHS_WARN(TEXT("ARSGameMode::BeginPlay — EnemySpawner를 찾지 못했습니다. 스테이지 시작 불가."));
		return;
	}

	// StageManager에 Spawner 등록 후 스테이지 시작
	// (InitPools는 StartStage 내부에서 DT_Stage.SpawnEnemyIDs 기반으로 호출됨)
	GET_WORLD_SUBSYSTEM(UStageManagerSubsystem, StageMgr);
	StageMgr->SetSpawner(Spawner);
	StageMgr->StartStage(DefaultStageID);
}
