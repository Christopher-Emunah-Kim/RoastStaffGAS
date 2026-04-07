// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_IsPhase2.generated.h"

/**
 * BTDecorator_IsPhase2
 *
 * - ABossEnemy::IsPhase2() 를 조회해 Phase2 활성 여부 판정.
 * - BT_BossEnemy Phase2 브랜치 진입 조건으로 사용.
 */
UCLASS()
class ROASTSTAFFGAS_API UBTDecorator_IsPhase2 : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_IsPhase2();

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
};
