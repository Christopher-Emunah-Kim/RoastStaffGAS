// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "RSGameSave.generated.h"

// ----------------------------------------------------------------------------
// FRSSettingsData — 플레이어 설정 데이터
// TODO: 설정 UI(EUIID::SETTING) 설계 완료 후 항목 확정 및 연동
// ----------------------------------------------------------------------------
USTRUCT(BlueprintType)
struct FRSSettingsData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float MasterVolume = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float BGMVolume = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float SFXVolume = 1.0f;
};

/**
 * URSSaveGame
 *
 * 플레이어 진행 데이터를 디스크에 영구 저장하는 세이브 오브젝트.
 * SaveGameSubsystem이 단독으로 생성·소유·갱신
 */
UCLASS()
class ROASTSTAFFGAS_API URSSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	/** 해금된 캐릭터 ID 목록 (DT_CharacterStatic FK) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save|Character")
	TArray<FName> UnlockedCharIDs;
	
	/** 클리어한 스테이지 ID 목록 (DT_Stage FK) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save|Stage")
	TArray<FName> ClearedStageIDs;

	/** 마지막으로 선택한 캐릭터 ID */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save|Character")
	FName LastSelectedCharacterID = NAME_None;

	/** 플레이어 설정 데이터 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save|Settings")
	FRSSettingsData SettingsData;

	/** 세이브 파일 버전 — 마이그레이션 판단용 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save|Meta")
	int32 SaveVersion = 1;

	// TODO(PLAN_SGS_Full): FStageRecord (클리어 시간/점수 등 상세 기록)
	// TODO(PLAN_SGS_Full): FTransactionState (재화 잔액, 누적 획득량 등)
	// TODO(PLAN_SGS_Full): TotalPlayCount / TotalClearCount / TotalGoldEarned
};
