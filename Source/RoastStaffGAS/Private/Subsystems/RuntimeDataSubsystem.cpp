// Fill out your copyright notice in the Description page of Project Settings.

#include "Subsystems/RuntimeDataSubsystem.h"
#include "Subsystems/SaveGameSubsystem.h"
#include "Subsystems/GameDataSubsystem.h"
#include "Data/RuntimeDataStructs.h"
#include "RoastStaffGAS.h"

void URuntimeDataSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	//의존성 순서에 따라 초기화 순서 권고
	Collection.InitializeDependency<UGameDataSubsystem>();
	Collection.InitializeDependency<USaveGameSubsystem>();

	GET_GI_SUBSYSTEM(USaveGameSubsystem, SaveSys);
	SaveSys->OnSaveGameLoadedDel.AddDynamic(this, &ThisClass::HandleSaveGameLoaded);

	//SGS가 항상 초기화가 먼저 되기떄문에, 기존 저장 정보 캐싱.
	if (SaveSys->IsSaveLoaded())
	{
		HandleSaveGameLoaded();
	}
}

void URuntimeDataSubsystem::HandleSaveGameLoaded()
{
	GET_GI_SUBSYSTEM(USaveGameSubsystem, SaveSys);

	SelectedCharacterID = SaveSys->GetLastSelectedCharacter();
	SettingsData        = SaveSys->GetSettingsData();
}

void URuntimeDataSubsystem::GatherPreloadCharacterAssets(TArray<FSoftObjectPath>& OutPaths)
{
	GET_GI_SUBSYSTEM(UGameDataSubsystem, GDS);
	
	if (!SelectedCharacterID.IsNone())
	{
		FCharacterPreloadBundle CharBundle;
		if (GDS->GetCharacterPreloadBundle(SelectedCharacterID, CharBundle))
		{
			if (!CharBundle.Mesh.IsNull())
			{
				OutPaths.AddUnique(CharBundle.Mesh.ToSoftObjectPath());
			}
			
			if (!CharBundle.AnimBP.IsNull())
			{
				OutPaths.AddUnique(CharBundle.AnimBP.ToSoftObjectPath());
			}
			
			for (const TSoftObjectPtr<UNiagaraSystem>& FX : CharBundle.SkillFXList)
			{
				if (!FX.IsNull())
				{
					OutPaths.AddUnique(FX.ToSoftObjectPath());
				}
			}
		}
	}
	else
	{
		KHS_WARN(TEXT("선택된 캐릭터 없음. 캐릭터 에셋 프리로드 생략."));
	}
}

void URuntimeDataSubsystem::GatherPreloadEnemyAssets(TArray<FSoftObjectPath>& OutPaths, bool bIncludeEnemies, const TArray<FName>& EnemyIDs)
{
	GET_GI_SUBSYSTEM(UGameDataSubsystem, GDS);
	
	if (!bIncludeEnemies)
	{
		KHS_INFO(TEXT("사용자 요청으로 EnemyAsset은 프리로드하지않음. "));
	}
	
	for (const FName& EnemyID : EnemyIDs)
	{
		FEnemyPreloadBundle EnemyBundle;
		if (GDS->GetEnemyPreloadBundle(EnemyID, EnemyBundle))
		{
			if (!EnemyBundle.EnemyClass.IsNull())
			{
				OutPaths.AddUnique(EnemyBundle.EnemyClass.ToSoftObjectPath());
			}
			if (!EnemyBundle.BehaviorTree.IsNull())
			{
				OutPaths.AddUnique(EnemyBundle.BehaviorTree.ToSoftObjectPath());
			}
		}
	}
}

// -----------------------------------------------------------------------------
// 선택 캐릭터
// -----------------------------------------------------------------------------
void URuntimeDataSubsystem::SetSelectedCharacter(FName CharID)
{
	SelectedCharacterID = CharID;
}

// -----------------------------------------------------------------------------
// 설정 데이터
// -----------------------------------------------------------------------------
void URuntimeDataSubsystem::SetSettingsData(const FRSSettingsData& NewSettings)
{
	SettingsData = NewSettings;

	// 설정 변경은 즉시 SGS에 동기화 + 디스크 저장
	GET_GI_SUBSYSTEM(USaveGameSubsystem, SaveSys);
	
	SaveSys->UpdateSettingsData(SettingsData);
	SaveSys->SaveGame();
}

// -----------------------------------------------------------------------------
// 직렬화
// -----------------------------------------------------------------------------
void URuntimeDataSubsystem::SerializeToPersistentData()
{
	GET_GI_SUBSYSTEM(USaveGameSubsystem, SaveSys);

	SaveSys->SetLastSelectedCharacter(SelectedCharacterID);
	SaveSys->UpdateSettingsData(SettingsData);
}

// -----------------------------------------------------------------------------
// 프리로드 에셋 수집
// -----------------------------------------------------------------------------
void URuntimeDataSubsystem::GatherPreloadAssets(TArray<FSoftObjectPath>& OutPaths, bool bIncludeEnemies, const TArray<FName>& EnemyIDs)
{
	// 선택 캐릭터 에셋 수집
	GatherPreloadCharacterAssets(OutPaths);

	// 에너미 에셋 수집
	GatherPreloadEnemyAssets(OutPaths, bIncludeEnemies,EnemyIDs);
	
}
