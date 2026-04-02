// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/RSGameMode.h"
#include "RoastStaffGAS.h"
#include "System/EnemySpawner.h"
#include "Subsystems/StageManagerSubsystem.h"
#include "Subsystems/SaveGameSubsystem.h"
#include "Subsystems/GameDataSubsystem.h"
#include "Subsystems/EquipmentSubsystem.h"
#include "Character/Player/RSPlayerState.h"
#include "Core/RSGameInstance.h"
#include "Data/DataTableStructs.h"
#include "Kismet/GameplayStatics.h"

void ARSGameMode::BeginPlay()
{
	Super::BeginPlay();

	// SGS에서 선택된 캐릭터 ID 조회
	GET_GI_SUBSYSTEM(USaveGameSubsystem, SGS);
	const FName CharID = SGS->GetLastSelectedCharacter();

	// 선택된 캐릭터 스탯을 PlayerState에 적용
	// (Possession은 BeginPlay 이전에 완료되므로 ASC는 초기화된 상태)
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		ARSPlayerState* PS = PC->GetPlayerState<ARSPlayerState>();
		if (PS)
		{
			PS->ApplyCharacterStats(CharID);
		}
		else
		{
			KHS_WARN(TEXT("PlayerState 조회 실패. 캐릭터 스탯 미적용."));
		}
	}

	// GI에서 다음 스테이지 ID 조회
	URSGameInstance* GI = Cast<URSGameInstance>(GetGameInstance());
	check(GI);
	const FName StageID = GI->GetNextStageID();
	if (StageID.IsNone())
	{
		KHS_WARN(TEXT("NextStageID가 NAME_None. 스테이지 시작 불가."));
		return;
	}

	// 맵에 배치된 EnemySpawner 탐색
	AEnemySpawner* Spawner = Cast<AEnemySpawner>(
		UGameplayStatics::GetActorOfClass(GetWorld(), AEnemySpawner::StaticClass()));

	if (!Spawner)
	{
		KHS_WARN(TEXT("EnemySpawner를 찾지 못했습니다. 스테이지 시작 불가."));
		return;
	}

	// DefaultWeapon 장착 (스테이지 진입 시 첫 무기 자동 세팅)
	InitDefaultWeapon(CharID);

	// StageManager에 Spawner 등록 후 스테이지 시작
	// (InitPools는 StartStage 내부에서 DT_Stage.SpawnEnemyIDs 기반으로 호출됨)
	GET_WORLD_SUBSYSTEM(UStageManagerSubsystem, StageMgr);
	StageMgr->SetSpawner(Spawner);
	StageMgr->StartStage(StageID);
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

	// EquipmentSubsystem은 PossessedBy 시점에 이미 InitializeSubsystem된 상태
	GET_GI_SUBSYSTEM(UEquipmentSubsystem, EquipSys);
	EquipSys->EquipWeapon(CharData.DefaultWeaponID);

	KHS_INFO(TEXT("DefaultWeapon 장착 완료 — CharID: %s / WeaponID: %s"),
		*CharID.ToString(), *CharData.DefaultWeaponID.ToString());
}
