// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/BTTask_RangedReposition.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Enemy/RangedEnemy.h"
#include "Character/Enemy/EliteEnemy.h"
#include "Character/Enemy/EnemyAIController.h"
#include "System/LoggingSystem.h"

UBTTask_RangedReposition::UBTTask_RangedReposition()
{
	NodeName       = TEXT("Ranged Reposition");
	bNotifyTick    = true;
	bNotifyTaskFinished = true;
}

// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────

static bool GetRangeParams(APawn* Pawn, float& OutPreferred, float& OutMax)
{
	if (ARangedEnemy* Ranged = Cast<ARangedEnemy>(Pawn))
	{
		OutPreferred = Ranged->GetPreferredRange();
		OutMax       = Ranged->GetMaxAttackRange();
		return true;
	}
	
	if (AEliteEnemy* Elite = Cast<AEliteEnemy>(Pawn))
	{
		OutPreferred = Elite->GetPreferredRange();
		OutMax       = Elite->GetMaxAttackRange();
		return true;
	}
	
	return false;
}

// ─────────────────────────────────────────────────────────────────────────────
// Task
// ─────────────────────────────────────────────────────────────────────────────

EBTNodeResult::Type UBTTask_RangedReposition::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

	float PreferredRange = 400.f;
	float MaxAttackRange = 800.f;
	if (!GetRangeParams(Pawn, PreferredRange, MaxAttackRange))
	{
		KHS_WARN(TEXT("[BTRepos] GetRangeParams FAILED — 타입 불일치. Pawn: %s / Class: %s"),
			*Pawn->GetName(), *Pawn->GetClass()->GetName());
		return EBTNodeResult::Failed;
	}

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
	{
		KHS_WARN(TEXT("Blackboard 없음."));
		return EBTNodeResult::Failed;
	}

	
	const FVector PawnLoc   = Pawn->GetActorLocation();
	const FVector PlayerLoc = BB->GetValueAsVector(AEnemyAIController::BBKey_PlayerLocation);
	FVector       Dir       = PawnLoc - PlayerLoc;
	Dir.Z                   = 0.f;
	const float   Dist      = Dir.Size2D();

	KHS_INFO(TEXT("[BTRepos] %s — Dist=%.0f Preferred=%.0f Max=%.0f"),
		*Pawn->GetName(), Dist, PreferredRange, MaxAttackRange);

	// 사거리 내 — 즉시 성공
	if (Dist >= PreferredRange && Dist <= MaxAttackRange)
	{
		return EBTNodeResult::Succeeded;
	}

	// 목표 위치 계산
	const FVector DirNorm = (Dist > KINDA_SMALL_NUMBER) ? Dir.GetSafeNormal2D() : FVector::ForwardVector;
	FVector TargetPos;
	if (Dist < PreferredRange)
	{
		// 너무 가까움 — PreferredRange 지점으로 후퇴
		TargetPos = PlayerLoc + DirNorm * PreferredRange;
	}
	else
	{
		// 너무 멀음 — MaxAttackRange 80% 지점으로 전진
		TargetPos = PlayerLoc + DirNorm * (MaxAttackRange * 0.8f);
	}

	FRangedRepositionMemory* Memory = reinterpret_cast<FRangedRepositionMemory*>(NodeMemory);
	Memory->TargetLocation = TargetPos;
	Memory->ElapsedTime    = 0.f;
	Memory->bNeedsMove     = true;

	AIC->MoveToLocation(TargetPos, AcceptanceRadius);
	return EBTNodeResult::InProgress;
}

void UBTTask_RangedReposition::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FRangedRepositionMemory* Memory = reinterpret_cast<FRangedRepositionMemory*>(NodeMemory);
	if (!Memory->bNeedsMove)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	AAIController* AIC = OwnerComp.GetAIOwner();
	if (!AIC || !AIC->GetPawn())
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	Memory->ElapsedTime += DeltaSeconds;

	const FVector PawnLoc = AIC->GetPawn()->GetActorLocation();
	const float   Dist    = FVector::Dist2D(PawnLoc, Memory->TargetLocation);

	if (Dist <= AcceptanceRadius || Memory->ElapsedTime >= MoveTimeout)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

void UBTTask_RangedReposition::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	AAIController* AIC = OwnerComp.GetAIOwner();
	if (AIC)
	{
		AIC->StopMovement();
	}
}

uint16 UBTTask_RangedReposition::GetInstanceMemorySize() const
{
	return sizeof(FRangedRepositionMemory);
}
