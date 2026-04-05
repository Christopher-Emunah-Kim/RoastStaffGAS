// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "RSGameMode.generated.h"

/**
 * ARSGameMode
 *
 * - 스테이지 시작 진입점. 초기화 순서를 보장하는 조율자 역할만 담당한다.
 * - Pool 초기화: UStageManagerSubsystem::StartStage → AEnemySpawner::InitPools()로 이관.
 * - 웨이브/스폰 로직: UStageManagerSubsystem에 위임.
 * - 스테이지 클리어/실패 판정 및 OUTGAME 복귀 처리
 */
UCLASS()
class ROASTSTAFFGAS_API ARSGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ARSGameMode();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:
	/** 플레이어 사망 시 외부(RSPlayerCharacter)에서 호출 */
	void OnStageFailed();
	
private:
	// BeginPlay 초기화 헬퍼
	void InitializePlayer(FName CharID);
	bool InitializeStage();
	void StartStageFlow();
	void InitDefaultWeapon(FName CharID);

	// 스테이지 클리어 판정 (매 프레임 TimeLimit 체크)
	void CheckStageClearCondition();

	// 스테이지 클리어 처리 (TimeLimit 초과 생존)
	void OnStageCleared();

	// 스테이지 종료 공통 처리 (결과 데이터 구성 + 저장 + OUTGAME 복귀)
	void EndStage(bool bCleared);

	// 결과 확인 후 OUTGAME 복귀 (현재는 즉시 복귀, MODULE-4에서 UI 연동)
	void OnResultConfirmed();

private:
	/** 스테이지 시작 시간 (GetWorld()->GetTimeSeconds() 기준) */
	float StageStartTime = 0.f;
	/** 현재 진행 중인 스테이지 ID */
	FName CurrentStageID = NAME_None;
	/** 스테이지 종료 플래그 (중복 호출 방지) */
	bool bIsStageEnded = false;
};
