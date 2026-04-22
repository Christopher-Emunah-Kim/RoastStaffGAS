// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/BTTask_FireProjectile.h"
#include "AIController.h"
#include "Character/Enemy/RangedEnemy.h"
#include "Character/Enemy/EliteEnemy.h"
#include "Character/Enemy/BossEnemy.h"
#include "System/LoggingSystem.h"

UBTTask_FireProjectile::UBTTask_FireProjectile()
{
	NodeName = TEXT("Fire Projectile");
}

EBTNodeResult::Type UBTTask_FireProjectile::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	//PRECONDITION
	AAIController* AIC = OwnerComp.GetAIOwner();
	if (!AIC)
	{
		KHS_WARN(TEXT("AIController 없음."));
		return EBTNodeResult::Failed;
	}

	APawn* Pawn = AIC->GetPawn();
	if (!Pawn)
	{
		KHS_WARN(TEXT("Pawn 없음."));
		return EBTNodeResult::Failed;
	}

	//타입별 발사
	//KHS_INFO(TEXT("[BTFire] ExecuteTask — %s"), *Pawn->GetName());

	if (ARangedEnemy* Ranged = Cast<ARangedEnemy>(Pawn))
	{
		Ranged->FireProjectile();
		return EBTNodeResult::Succeeded;
	}

	if (AEliteEnemy* Elite = Cast<AEliteEnemy>(Pawn))
	{
		Elite->FireProjectile();
		return EBTNodeResult::Succeeded;
	}

	if (ABossEnemy* Boss = Cast<ABossEnemy>(Pawn))
	{
		Boss->FireSpreadProjectile();
		return EBTNodeResult::Succeeded;
	}

	KHS_WARN(TEXT("지원하지 않는 에너미 타입: %s"), *Pawn->GetName());
	return EBTNodeResult::Failed;
}
