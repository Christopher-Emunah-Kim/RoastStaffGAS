// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/BTTask_MeleeCharge.h"
#include "AIController.h"
#include "Character/Enemy/EliteEnemy.h"
#include "System/LoggingSystem.h"

UBTTask_MeleeCharge::UBTTask_MeleeCharge()
{
	NodeName    = TEXT("Melee Charge");
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_MeleeCharge::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIC = OwnerComp.GetAIOwner();
	if (!AIC)
	{
		KHS_WARN(TEXT("AIController 없음."));
		return EBTNodeResult::Failed;
	}

	AEliteEnemy* Elite = Cast<AEliteEnemy>(AIC->GetPawn());
	if (!Elite)
	{
		KHS_WARN(TEXT("EliteEnemy 아님: %s"), *AIC->GetPawn()->GetName());
		return EBTNodeResult::Failed;
	}

	FMeleeChargeMemory* Memory = reinterpret_cast<FMeleeChargeMemory*>(NodeMemory);
	Memory->ElapsedTime        = 0.f;

	Elite->MeleeCharge();
	return EBTNodeResult::InProgress;
}

void UBTTask_MeleeCharge::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FMeleeChargeMemory* Memory = reinterpret_cast<FMeleeChargeMemory*>(NodeMemory);
	Memory->ElapsedTime += DeltaSeconds;

	AAIController* AIC   = OwnerComp.GetAIOwner();
	AEliteEnemy*   Elite = AIC ? Cast<AEliteEnemy>(AIC->GetPawn()) : nullptr;

	if (!Elite || !Elite->IsCharging() || Memory->ElapsedTime >= MaxWaitTime)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

uint16 UBTTask_MeleeCharge::GetInstanceMemorySize() const
{
	return sizeof(FMeleeChargeMemory);
}
