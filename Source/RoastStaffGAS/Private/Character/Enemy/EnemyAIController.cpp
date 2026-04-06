// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Enemy/EnemyAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "GAS/Tags/RSGameplayTags.h"
#include "System/LoggingSystem.h"
#include "Kismet/GameplayStatics.h"


const FName AEnemyAIController::BBKey_PlayerLocation = TEXT("PlayerLocation");
const FName AEnemyAIController::BBKey_bPlayerDead    = TEXT("bPlayerDead");
const FName AEnemyAIController::BBKey_bIsPhase2      = TEXT("bIsPhase2");

AEnemyAIController::AEnemyAIController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}

void AEnemyAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	//플레이어가 죽거나, 유효하지않으면 false.
	//플레이어가 유효한 경우 tick으로 위치 추적.
	if (!UpdatePlayerInfo())
	{
		return;
	}
}


bool AEnemyAIController::UpdatePlayerInfo()
{
	UBlackboardComponent* BB = GetBlackboardComponent();
	if (!BB)
	{
		return false;
	}

	// 플레이어 폰이 캐싱되지 않은 경우 재탐색 시도
	if (!CachedPlayerPawn.IsValid())
	{
		APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
		if (!PlayerPawn)
		{
			KHS_WARN(TEXT("%s — CANNOT FIND PLAYER PAWN."), *GetName());
			return false;
		}
		CachedPlayerPawn = PlayerPawn;
	}

	// 플레이어 사망 여부 갱신
	const bool bDead = IsPlayerDead();
	BB->SetValueAsBool(BBKey_bPlayerDead, bDead);

	// 생존 중일 때만 위치 갱신
	if (!bDead)
	{
		BB->SetValueAsVector(BBKey_PlayerLocation, CachedPlayerPawn->GetActorLocation());
	} 
	return true;
}


bool AEnemyAIController::IsPlayerDead() const
{
	if (!CachedPlayerPawn.IsValid())
	{
		return false;
	}

	IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(CachedPlayerPawn.Get());
	if (!ASI)
	{
		return false;
	}

	UAbilitySystemComponent* PlayerASC = ASI->GetAbilitySystemComponent();
	if (!PlayerASC)
	{
		return false;
	}

	//플레이어가 죽음 상태(State.Dead)이면 true
	return PlayerASC->HasMatchingGameplayTag(RSTags::State_Dead);
}

void AEnemyAIController::StartAI(UBehaviorTree* BehaviorTree)
{
	if (!ensureMsgf(BehaviorTree, TEXT("%s — BehaviorTree IS NULL."), *GetName()))
	{
		return;
	}

	bool bSuccess = RunBehaviorTree(BehaviorTree);
	if (!bSuccess)
	{
		KHS_WARN(TEXT("%s — BehaviorTree 실행 실패."), *GetName());
		check(false);
		return;
	}

	UBlackboardComponent* BB = GetBlackboardComponent();
	if (BB)
	{
		BB->SetValueAsBool(BBKey_bPlayerDead, false);
	}

	KHS_INFO(TEXT("%s — BehaviorTree 실행 완료."), *GetName());
}

void AEnemyAIController::SetInitialTargetLocation(const FVector& Location)
{
	UBlackboardComponent* BB = GetBlackboardComponent();
	if (!BB)
	{
		KHS_WARN(TEXT("%s — SetInitialTargetLocation: Blackboard가 없음"), *GetName());
		return;
	}
	BB->SetValueAsVector(BBKey_PlayerLocation, Location);
	BB->SetValueAsBool(BBKey_bPlayerDead, false);
}

void AEnemyAIController::StopAI()
{
	if (UBrainComponent* Brain = GetBrainComponent())
	{
		Brain->StopLogic(TEXT("Pooled — deactivated"));
	}
	StopMovement();
	KHS_INFO(TEXT("%s — AI 중단."), *GetName());
}

void AEnemyAIController::PauseAI()
{
	if (UBrainComponent* Brain = GetBrainComponent())
	{
		Brain->PauseLogic(TEXT("Boss phase transition"));
	}
	StopMovement();
	KHS_DEBUG(TEXT("%s — AI 일시 중단 (페이즈 전환)."), *GetName());
}

void AEnemyAIController::ResumeAI()
{
	if (UBrainComponent* Brain = GetBrainComponent())
	{
		Brain->ResumeLogic(TEXT("Boss phase transition complete"));
	}
	KHS_DEBUG(TEXT("%s — AI 재개 (페이즈2 활성)."), *GetName());
}
