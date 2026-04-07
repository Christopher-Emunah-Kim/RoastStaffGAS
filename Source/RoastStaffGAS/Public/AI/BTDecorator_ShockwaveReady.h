// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_ShockwaveReady.generated.h"

/**
 * BTDecorator_ShockwaveReady
 *
 * - ABossEnemy::IsShockwaveReady() 를 조회해 쿨타임 만료 여부 판정.
 * - true → 데코레이터 통과 / false → 브랜치 차단.
 */
UCLASS()
class ROASTSTAFFGAS_API UBTDecorator_ShockwaveReady : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_ShockwaveReady();

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
};
