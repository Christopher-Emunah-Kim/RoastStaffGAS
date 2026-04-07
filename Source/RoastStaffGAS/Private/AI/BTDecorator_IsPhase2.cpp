// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/BTDecorator_IsPhase2.h"
#include "AIController.h"
#include "Character/Enemy/BossEnemy.h"

UBTDecorator_IsPhase2::UBTDecorator_IsPhase2()
{
	NodeName = TEXT("Is Phase 2");
}

bool UBTDecorator_IsPhase2::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	AAIController* AIC = OwnerComp.GetAIOwner();
	if (!AIC)
	{
		return false;
	}

	ABossEnemy* Boss = Cast<ABossEnemy>(AIC->GetPawn());
	return Boss && Boss->IsPhase2();
}
