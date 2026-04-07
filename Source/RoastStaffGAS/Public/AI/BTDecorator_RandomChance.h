// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_RandomChance.generated.h"

/**
 * BTDecorator_RandomChance
 *
 * - [0, 1) 구간 랜덤값이 ChanceProbability 미만이면 조건 통과.
 * - EliteEnemy 돌진 확률 분기 등에 사용.
 */
UCLASS()
class ROASTSTAFFGAS_API UBTDecorator_RandomChance : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_RandomChance();

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

private:
	/** 조건 통과 확률 [0.0, 1.0] */
	UPROPERTY(EditAnywhere, Category = "MY|AI|RandomChance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ChanceProbability = 0.3f;
};
