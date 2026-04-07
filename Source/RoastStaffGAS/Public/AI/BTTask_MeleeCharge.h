// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_MeleeCharge.generated.h"

/** BTTask_MeleeCharge용 NodeMemory */
struct FMeleeChargeMemory
{
	float ElapsedTime = 0.f;
};

/**
 * BTTask_MeleeCharge
 *
 * - 엘리트 에너미 근접 공격 Task
 * - AEliteEnemy::MeleeCharge() 호출 후 돌진 완료(IsCharging() == false)까지 대기.
 * - MaxWaitTime 초과 시 강제 Succeed — BT 블로킹 방지.
 */
UCLASS()
class ROASTSTAFFGAS_API UBTTask_MeleeCharge : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_MeleeCharge();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual uint16 GetInstanceMemorySize() const override;

private:
	/** 돌진 완료 폴링 최대 대기 시간 — EliteEnemy 타임아웃보다 여유 있게 설정 */
	UPROPERTY(EditAnywhere, Category = "MY|AI|MeleeCharge")
	float MaxWaitTime = 5.f;
};
