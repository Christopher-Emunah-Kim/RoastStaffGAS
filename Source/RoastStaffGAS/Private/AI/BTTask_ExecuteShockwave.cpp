// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/BTTask_ExecuteShockwave.h"
#include "AIController.h"
#include "Character/Enemy/BossEnemy.h"
#include "System/LoggingSystem.h"

UBTTask_ExecuteShockwave::UBTTask_ExecuteShockwave()
{
	NodeName    = TEXT("Execute Shockwave");
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_ExecuteShockwave::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	//PRECONDITION
	AAIController* AIC = OwnerComp.GetAIOwner();
	if (!AIC)
	{
		KHS_WARN(TEXT("AIController 없음."));
		return EBTNodeResult::Failed;
	}

	ABossEnemy* Boss = Cast<ABossEnemy>(AIC->GetPawn());
	if (!Boss)
	{
		KHS_WARN(TEXT("BossEnemy 아님."));
		return EBTNodeResult::Failed;
	}

	FExecuteShockwaveMemory* Memory = reinterpret_cast<FExecuteShockwaveMemory*>(NodeMemory);
	Memory->ElapsedTime        = 0.f;
	Memory->bShockwaveExecuted = false;

	// PrepareTime이 0 이하면 즉시 실행
	if (Boss->GetShockwavePrepareTime() <= 0.f)
	{
		Boss->ExecuteShockwave();
		Boss->MarkShockwaveUsed();
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::InProgress;
}

void UBTTask_ExecuteShockwave::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FExecuteShockwaveMemory* Memory = reinterpret_cast<FExecuteShockwaveMemory*>(NodeMemory);
	if (Memory->bShockwaveExecuted)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	AAIController* AIC = OwnerComp.GetAIOwner();
	if (!AIC)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	ABossEnemy* Boss = Cast<ABossEnemy>(AIC->GetPawn());
	if (!Boss)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	Memory->ElapsedTime += DeltaSeconds;
	
	if (Memory->ElapsedTime >= Boss->GetShockwavePrepareTime())
	{
		Boss->ExecuteShockwave();
		Boss->MarkShockwaveUsed();
		Memory->bShockwaveExecuted = true;
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

uint16 UBTTask_ExecuteShockwave::GetInstanceMemorySize() const
{
	return sizeof(FExecuteShockwaveMemory);
}
