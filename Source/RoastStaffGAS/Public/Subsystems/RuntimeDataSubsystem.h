// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Core/RSGameSave.h"
#include "RuntimeDataSubsystem.generated.h"

/**
 * URuntimeDataSubsystem (RDS)
 *
 * 런타임 선택 데이터 관리 시스템
 * - 세이브 시스템의 OnSaveGameLoadedDel 구독 → 세이브 복원 후 메모리 캐시 구성
 * - SelectedCharacterID, SettingsData를 세션 내 단일 창구로 제공
 * - GatherPreloadAssets(): 트랜지션 시 로드할 에셋 경로 목록을 GDS 경유로 수집
 * - SerializeToPersistentData(): 스테이지 진입 직전 현재 상태를 SGS에 동기화
 */
UCLASS()
class ROASTSTAFFGAS_API URuntimeDataSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// -------------------------------------------------------------------------
	// 선택 캐릭터
	// -------------------------------------------------------------------------
	FORCEINLINE FName GetSelectedCharacterID() const { return SelectedCharacterID; }

	/** 캐릭터 선택 확정 시 호출. 메모리 업데이트만 수행 (디스크 저장은 스테이지 진입 시 일괄). */
	void SetSelectedCharacter(FName CharID);

	// -------------------------------------------------------------------------
	// 설정 데이터
	// -------------------------------------------------------------------------
	FORCEINLINE FRSSettingsData GetSettingsData() const { return SettingsData; }

	/** 설정 변경 시 호출. SGS::UpdateSettingsData + SaveGame 즉시 처리 (설정 변경 즉시 저장 정책). */
	void SetSettingsData(const FRSSettingsData& NewSettings);

	// -------------------------------------------------------------------------
	// 직렬화 (스테이지 진입 직전 호출)
	// -------------------------------------------------------------------------
	/** 현재 메모리 상태를 SGS에 동기화. SGS::SaveGame()은 호출자가 별도 처리. */
	void SerializeToPersistentData();

	// -------------------------------------------------------------------------
	// 프리로드 에셋 수집
	// -------------------------------------------------------------------------
	/**
	 * 트랜지션 로딩 시 프리로드할 에셋 경로 목록을 OutPaths에 추가.
	 * @param bIncludeEnemies  true이면 EnemyIDs 목록의 에너미 에셋도 포함
	 * @param EnemyIDs         스테이지 웨이브에 등장할 에너미 ID 목록
	 */
	void GatherPreloadAssets(TArray<FSoftObjectPath>& OutPaths, bool bIncludeEnemies, const TArray<FName>& EnemyIDs);

	
private:
	/** SGS::OnSaveGameLoadedDel 수신 콜백 — SelectedCharacterID + SettingsData 복원 */
	UFUNCTION()
	void HandleSaveGameLoaded();

	//프리로드 에셋 수집 헬퍼
	void GatherPreloadCharacterAssets(TArray<FSoftObjectPath>& OutPaths);
	void GatherPreloadEnemyAssets(TArray<FSoftObjectPath>& OutPaths, bool bIncludeEnemies, const TArray<FName>& EnemyIDs);
	
	FName           SelectedCharacterID = NAME_None;
	FRSSettingsData SettingsData;
};
