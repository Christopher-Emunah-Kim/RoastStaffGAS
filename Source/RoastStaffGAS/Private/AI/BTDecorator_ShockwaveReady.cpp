// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/BTDecorator_ShockwaveReady.h"
#include "AIController.h"
#include "Character/Enemy/BossEnemy.h"

UBTDecorator_ShockwaveReady::UBTDecorator_ShockwaveReady()
{
	NodeName = TEXT("Shockwave Ready");
}

bool UBTDecorator_ShockwaveReady::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	AAIController* AIC = OwnerComp.GetAIOwner();
	if (!AIC)
	{
		return false;
	}

	ABossEnemy* Boss = Cast<ABossEnemy>(AIC->GetPawn());
	return Boss && Boss->IsShockwaveReady();
}
