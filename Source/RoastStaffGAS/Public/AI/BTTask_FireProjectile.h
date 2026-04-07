// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_FireProjectile.generated.h"

/**
 * BTTask_FireProjectile
 *
 * - ARangedEnemy / AEliteEnemy → FireProjectile()
 * - ABossEnemy (Phase2)        → FireSpreadProjectile()
 * - 해당하는 캐스트가 없으면 Failed.
 */
UCLASS()
class ROASTSTAFFGAS_API UBTTask_FireProjectile : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_FireProjectile();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
