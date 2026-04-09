// Fill out your copyright notice in the Description page of Project Settings.

#include "Subsystems/SaveGameSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "RoastStaffGAS.h"

void USaveGameSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	LoadGame();
	OnSaveGameLoadedDel.Broadcast();
}

void USaveGameSubsystem::Deinitialize()
{
	// 앱 종료 시 메모리 캐시 최종 디스크 저장
	SaveGame();

	Super::Deinitialize();
}

// -----------------------------------------------------------------------------
// 저장 / 로드
// -----------------------------------------------------------------------------
void USaveGameSubsystem::SaveGame()
{
	if (!CachedSaveGame)
	{
		KHS_WARN(TEXT("CachedSaveGame이 nullptr입니다. 저장 건너뜀."));
		return;
	}

	const bool bSuccess = UGameplayStatics::SaveGameToSlot(CachedSaveGame, SaveSlotName, SaveUserIndex);
	if (!bSuccess)
	{
		KHS_WARN(TEXT("디스크 저장 실패 (슬롯: %s). 메모리 캐시는 유지됩니다."), SaveSlotName);
	}
}

void USaveGameSubsystem::LoadGame()
{
	if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, SaveUserIndex))
	{
		USaveGame* Loaded = UGameplayStatics::LoadGameFromSlot(SaveSlotName, SaveUserIndex);
		CachedSaveGame = Cast<URSSaveGame>(Loaded);

		if (!CachedSaveGame)
		{
			// 타입 불일치 — 신규 생성으로 복구
			KHS_WARN(TEXT("세이브 파일 타입 불일치. 신규 생성으로 복구."));
			CachedSaveGame = Cast<URSSaveGame>(UGameplayStatics::CreateSaveGameObject(URSSaveGame::StaticClass()));
			SaveGame();
		}
	}
	else
	{
		// 최초 실행 — 신규 생성 후 즉시 저장
		CachedSaveGame = Cast<URSSaveGame>(UGameplayStatics::CreateSaveGameObject(URSSaveGame::StaticClass()));
		SaveGame();
	}

	bIsSaveLoaded = (CachedSaveGame != nullptr);
}

// -----------------------------------------------------------------------------
// 조회
// -----------------------------------------------------------------------------
bool USaveGameSubsystem::IsStageCleared(FName StageID) const
{
	if (!CachedSaveGame)
	{
		return false;
	}

	return CachedSaveGame->ClearedStageIDs.Contains(StageID);
}

bool USaveGameSubsystem::IsCharacterUnlocked(FName CharID) const
{
	if (!CachedSaveGame)
	{
		return false;
	}

	return CachedSaveGame->UnlockedCharIDs.Contains(CharID);
}

FName USaveGameSubsystem::GetLastSelectedCharacter() const
{
	if (!CachedSaveGame)
	{
		return NAME_None;
	}

	return CachedSaveGame->LastSelectedCharacterID;
}

FRSSettingsData USaveGameSubsystem::GetSettingsData() const
{
	if (!CachedSaveGame)
	{
		return FRSSettingsData{};
	}

	return CachedSaveGame->SettingsData;
}

FStageRecord USaveGameSubsystem::GetStageRecord(FName StageID) const
{
	if (!CachedSaveGame)
	{
		return FStageRecord{};
	}

	const FStageRecord* Found = CachedSaveGame->StageRecords.Find(StageID);
	return Found ? *Found : FStageRecord{};
}

// -----------------------------------------------------------------------------
// 갱신
// -----------------------------------------------------------------------------
void USaveGameSubsystem::AddClearedStage(FName StageID)
{
	if (!CachedSaveGame)
	{
		KHS_WARN(TEXT("CachedSaveGame nullptr. StageID: %s"), *StageID.ToString());
		return;
	}

	CachedSaveGame->ClearedStageIDs.AddUnique(StageID);
}

void USaveGameSubsystem::UnlockCharacter(FName CharID)
{
	if (!CachedSaveGame)
	{
		KHS_WARN(TEXT("CachedSaveGame nullptr. CharID: %s"), *CharID.ToString());
		return;
	}

	CachedSaveGame->UnlockedCharIDs.AddUnique(CharID);
}

void USaveGameSubsystem::SetLastSelectedCharacter(FName CharID)
{
	if (!CachedSaveGame)
	{
		KHS_WARN(TEXT("CachedSaveGame nullptr. CharID: %s"), *CharID.ToString());
		return;
	}

	CachedSaveGame->LastSelectedCharacterID = CharID;
}

FName USaveGameSubsystem::GetLastPlayedStageID() const
{
	if (!CachedSaveGame)
	{
		return NAME_None;
	}

	return CachedSaveGame->LastPlayedStageID;
}

void USaveGameSubsystem::SetLastPlayedStageID(FName StageID)
{
	if (!CachedSaveGame)
	{
		KHS_WARN(TEXT("CachedSaveGame nullptr. StageID: %s"), *StageID.ToString());
		return;
	}

	CachedSaveGame->LastPlayedStageID = StageID;
}

void USaveGameSubsystem::UpdateSettingsData(const FRSSettingsData& NewSettings)
{
	if (!CachedSaveGame)
	{
		KHS_WARN(TEXT("CachedSaveGame nullptr. 설정 업데이트 건너뜀."));
		return;
	}

	CachedSaveGame->SettingsData = NewSettings;
}

void USaveGameSubsystem::UpdateStageRecord(FName StageID, const FStageResultData& ResultData)
{
	if (!CachedSaveGame)
	{
		KHS_WARN(TEXT("CachedSaveGame nullptr. StageID: %s"), *StageID.ToString());
		return;
	}

	// 기존 기록 조회 또는 신규 생성
	FStageRecord& Record = CachedSaveGame->StageRecords.FindOrAdd(StageID);

	// BestSurvivalTime 갱신 (더 크면)
	if (ResultData.SurvivalTime > Record.BestSurvivalTime)
	{
		Record.BestSurvivalTime = ResultData.SurvivalTime;
	}

	// BestKillCount 갱신 (더 크면)
	if (ResultData.KillCount > Record.BestKillCount)
	{
		Record.BestKillCount = ResultData.KillCount;
	}

	// bIsCleared 갱신 (한 번 true가 되면 false로 복귀 금지)
	if (ResultData.bCleared)
	{
		Record.bIsCleared = true;

		// ClearedStageIDs 목록에도 추가
		CachedSaveGame->ClearedStageIDs.AddUnique(StageID);
	}

	// PlayCount 증가
	Record.PlayCount++;

	// 디스크 즉시 저장
	SaveGame();

	KHS_INFO(TEXT("스테이지 기록 갱신: %s | 생존: %.1f | 처치: %d | 클리어: %s | 플레이: %d"),
		*StageID.ToString(),
		Record.BestSurvivalTime,
		Record.BestKillCount,
		Record.bIsCleared ? TEXT("O") : TEXT("X"),
		Record.PlayCount);
}
