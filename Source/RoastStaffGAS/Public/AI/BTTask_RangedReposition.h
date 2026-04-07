// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_RangedReposition.generated.h"

/** BTTask_RangedReposition용 NodeMemory */
struct FRangedRepositionMemory
{
	FVector TargetLocation = FVector::ZeroVector;
	float   ElapsedTime    = 0.f;
	bool    bNeedsMove     = false;
};

/**
 * BTTask_RangedReposition
 *
 * - 에너미-플레이어 거리 기반 재배치.
 * - 거리 < PreferredRange  → 후퇴 이동 (플레이어 반대 방향으로 PreferredRange 확보)
 * - 거리 > MaxAttackRange  → 전진 이동 (MaxAttackRange 80% 지점까지)
 * - 사거리 내              → 즉시 Succeeded
 * - ARangedEnemy / AEliteEnemy 공용.
 */
UCLASS()
class ROASTSTAFFGAS_API UBTTask_RangedReposition : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_RangedReposition();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;
	virtual uint16 GetInstanceMemorySize() const override;

private:
	/** 도착 판정 반경 */
	UPROPERTY(EditAnywhere, Category = "MY|AI|Reposition")
	float AcceptanceRadius = 50.f;

	/** 이동 타임아웃 (초) — 경로 막힘 시 강제 Succeed로 BT 진행 */
	UPROPERTY(EditAnywhere, Category = "MY|AI|Reposition")
	float MoveTimeout = 3.f;
};
