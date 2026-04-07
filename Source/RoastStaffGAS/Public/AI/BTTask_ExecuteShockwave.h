// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ExecuteShockwave.generated.h"

/** BTTask_ExecuteShockwave용 NodeMemory */
struct FExecuteShockwaveMemory
{
	float ElapsedTime       = 0.f;
	bool  bShockwaveExecuted = false;
};

/**
 * BTTask_ExecuteShockwave
 *
 * - PrepareTime(선딜) 대기 후 ABossEnemy::ExecuteShockwave() 호출.
 * - 완료 후 MarkShockwaveUsed() → 쿨타임 시작.
 * - PrepareTime은 ABossEnemy::GetShockwavePrepareTime()에서 조회.
 */
UCLASS()
class ROASTSTAFFGAS_API UBTTask_ExecuteShockwave : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_ExecuteShockwave();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual uint16 GetInstanceMemorySize() const override;
};
