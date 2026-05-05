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
	bool UpdatePlayerInfo();
	bool IsPlayerDead() const;

	/**
	 * 플레이어와의 거리에 따라 CMC·SkeletalMesh 틱 간격 동적 조정.
	 * AIC Tick(0.1s 간격)에서 호출 — 근거리 전투 품질 유지, 원거리 CPU 절감.
	 */
	void AdjustPawnTickRates(APawn* ControlledPawn, float DistToPlayer);

public:
	// Blackboard 키 이름 상수
	static const FName BBKey_PlayerLocation;
	static const FName BBKey_bPlayerDead;
	static const FName BBKey_bIsPhase2;
	
private:
	// tick 체크용 캐싱
	TWeakObjectPtr<APawn> CachedPlayerPawn;

	// ── Tick 최적화 파라미터 (BP에서 에너미 종류별 조정 가능) ──────────────────
	/** 근거리 임계값 (cm) — 이하에서는 CMC·Anim 틱을 매 프레임으로 유지 */
	UPROPERTY(EditDefaultsOnly, Category = "MY|AI|TickOpt")
	float NearThreshold = 3000.f;
	/** 원거리 임계값 (cm) — 이상에서는 FarCMC/AnimTickInterval 적용 */
	UPROPERTY(EditDefaultsOnly, Category = "MY|AI|TickOpt")
	float FarThreshold = 5000.f;
	/** 중거리 CMC 틱 간격 (초) */
	UPROPERTY(EditDefaultsOnly, Category = "MY|AI|TickOpt")
	float MidCMCTickInterval = 0.033f;
	/** 원거리 CMC 틱 간격 (초) */
	UPROPERTY(EditDefaultsOnly, Category = "MY|AI|TickOpt")
	float FarCMCTickInterval = 0.05f;
	/** 중거리 SkeletalMesh 애니메이션 틱 간격 (초) */
	UPROPERTY(EditDefaultsOnly, Category = "MY|AI|TickOpt")
	float MidAnimTickInterval = 0.05f;
	/** 원거리 SkeletalMesh 애니메이션 틱 간격 (초) */
	UPROPERTY(EditDefaultsOnly, Category = "MY|AI|TickOpt")
	float FarAnimTickInterval = 0.05f;
};
