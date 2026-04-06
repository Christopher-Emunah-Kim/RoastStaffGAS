// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIController.generated.h"

/**
 * UEnemyAIController
 *
 * - 에너미 공통 AI 컨트롤러.
 * - BT 실행 및 Blackboard 키 갱신 담당.
 */

class UBehaviorTree;
class UBlackboardComponent;


UCLASS()
class ROASTSTAFFGAS_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	AEnemyAIController();

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaTime) override;
	
private:
	bool UpdatePlayerInfo();
	bool IsPlayerDead() const;
	
public:
	void StartAI(UBehaviorTree* BehaviorTree);

	/** BT 시작 전 Blackboard에 초기 플레이어 위치를 즉시 설정 — Tick 대기 없이 첫 틱부터 유효한 목표 제공 */
	void SetInitialTargetLocation(const FVector& Location);

	/** 풀 반납 시 BT 로직 중단 및 이동 정지 */
	void StopAI();
	/** 보스 페이즈 전환 시 BT 일시 중단 (StopLogic이 아닌 PauseLogic — 재개 가능) */
	void PauseAI();
	/** PauseAI 이후 재개 — 페이즈 전환 완료 시 호출 */
	void ResumeAI();

private:
	// tick 체크용 캐싱
	TWeakObjectPtr<APawn> CachedPlayerPawn;

public:
	// Blackboard 키 이름 상수
	static const FName BBKey_PlayerLocation;
	static const FName BBKey_bPlayerDead;
	static const FName BBKey_bIsPhase2;
};
