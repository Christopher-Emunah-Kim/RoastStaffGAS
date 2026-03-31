// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Data/EnumUITypes.h"
#include "RSGameInstance.generated.h"

/**
 * URSGameInstance
 *
 * 레벨 전환 허브. 다음 목적지 레벨명/스테이지 ID를 보관하고
 * OpenNextLevelByName → TRANSITION 경유 → OpenNextLevelLatent 패턴으로 전환한다.
 *
 * - OpenNextLevelByName : UI 리셋 후 0.1s 타이머로 TRANSITION 레벨 열기
 * - OpenNextLevelLatent : MapSettings.LevelMap[NextLevelName] 경로로 최종 목적지 이동
 * - OpenNextStage       : 스테이지 ID 저장 후 OpenNextLevelByName(STAGE) 위임
 */
UCLASS()
class ROASTSTAFFGAS_API URSGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	// =========================================================================
	// 레벨 전환 API
	// =========================================================================
	/**
	 * 다음 레벨로 이동 요청.
	 * UIManager 상태 전체 리셋 후 0.1s 뒤 TRANSITION 레벨로 이동.
	 */
	UFUNCTION(BlueprintCallable, Category = "RS|LevelTransition")
	void OpenNextLevelByName(ELevelName Level);

	/**
	 * TRANSITION 레벨에서 호출. MapSettings.LevelMap[NextLevelName] 경로로 최종 레벨 이동.
	 * NextLevelName에 매핑된 경로가 없으면 에러 로그 후 중단.
	 */
	UFUNCTION(BlueprintCallable, Category = "RS|LevelTransition")
	void OpenNextLevelLatent();

	/**
	 * 스테이지 진입 요청.
	 * NextStageID 저장 후 OpenNextLevelByName(STAGE) 위임.
	 */
	UFUNCTION(BlueprintCallable, Category = "RS|LevelTransition")
	void OpenNextStage(FName StageID);

	// =========================================================================
	// 상태 Getter/Setter
	// =========================================================================
	FORCEINLINE ELevelName GetNextLevelName() const { return NextLevelName; }
	FORCEINLINE void       SetNextLevelName(ELevelName Level) { NextLevelName = Level; }

	FORCEINLINE FName GetNextStageID() const { return NextStageID; }
	FORCEINLINE void  SetNextStageID(FName StageID) { NextStageID = StageID; }

private:
	/** OpenNextLevelByName이 예약한 레벨 전환 실행 — 0.1s 타이머 콜백 */
	void DoOpenTransitionLevel();
	
	
private:
	/** OpenNextLevelByName이 설정, TRANSITION 레벨 완료 후 OpenNextLevelLatent가 소비 */
	ELevelName NextLevelName = ELevelName::INTRO;
	/** OpenNextStage가 설정, 스테이지 로딩 시 StageManagerSubsystem이 소비 */
	FName NextStageID = NAME_None;

};
