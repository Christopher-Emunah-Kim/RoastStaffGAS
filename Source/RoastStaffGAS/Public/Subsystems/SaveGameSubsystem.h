// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Core/RSGameSave.h"
#include "SaveGameSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSaveGameLoaded);

/**
 * USaveGameSubsystem
 *
 * 세이브 데이터의 로드·저장·조회를 담당하는 단일 창구.
 * - Initialize()에서 LoadGame()을 호출하고 OnSaveGameLoadedDel을 브로드캐스트
 * - 데이터 변경(AddClearedStage 등)은 메모리 캐시만 갱신하며, 실제 디스크 저장은 스테이지 진입 직전 SaveGame()으로 일괄 처리.
 * - Deinitialize()에서 SaveGame()을 호출해 앱 종료 시 최종 저장을 보장
 */
UCLASS()
class ROASTSTAFFGAS_API USaveGameSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// -------------------------------------------------------------------------
	// 저장 / 로드
	// -------------------------------------------------------------------------
	/** 현재 메모리 캐시를 디스크에 기록. */
	void SaveGame();
	/** 디스크에서 세이브 데이터를 로드. 파일이 없으면 신규 생성 후 즉시 저장. */
	void LoadGame();

	// -------------------------------------------------------------------------
	// 조회
	// -------------------------------------------------------------------------
	UFUNCTION(BlueprintCallable, Category = "RS|Save")
	bool IsStageCleared(FName StageID) const;

	UFUNCTION(BlueprintCallable, Category = "RS|Save")
	bool IsCharacterUnlocked(FName CharID) const;

	UFUNCTION(BlueprintCallable, Category = "RS|Save")
	FName GetLastSelectedCharacter() const;

	// -------------------------------------------------------------------------
	// 갱신 (디스크 즉시 저장 안 함 — 진입 직전 SaveGame() 일괄 처리)
	// -------------------------------------------------------------------------
	/** 스테이지 클리어 시 호출. 이미 기록된 ID면 무시. */
	void AddClearedStage(FName StageID);

	/** 캐릭터 해금 시 호출. 이미 해금된 ID면 무시. */
	void UnlockCharacter(FName CharID);

	/** 선택 캐릭터 변경 시 호출. */
	void SetLastSelectedCharacter(FName CharID);

	// -------------------------------------------------------------------------
	// 이벤트
	// -------------------------------------------------------------------------
	
	FORCEINLINE bool IsSaveLoaded() const { return bIsSaveLoaded; }
	
	/** LoadGame() 완료 후 브로드캐스트 — RuntimeDataSubsystem 등이 구독 */
	UPROPERTY(BlueprintAssignable, Category = "RS|Save")
	FOnSaveGameLoaded OnSaveGameLoadedDel;

private:
	static constexpr const TCHAR* SaveSlotName = TEXT("GameSave");
	static constexpr int32 SaveUserIndex = 0;

	UPROPERTY()
	TObjectPtr<URSSaveGame> CachedSaveGame = nullptr;

	bool bIsSaveLoaded = false;
};
