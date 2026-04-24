// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Enemy/EnemyAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "BrainComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GAS/Tags/RSGameplayTags.h"
#include "System/LoggingSystem.h"
#include "Kismet/GameplayStatics.h"


const FName AEnemyAIController::BBKey_PlayerLocation = TEXT("PlayerLocation");
const FName AEnemyAIController::BBKey_bPlayerDead    = TEXT("bPlayerDead");
const FName AEnemyAIController::BBKey_bIsPhase2      = TEXT("bIsPhase2");

AEnemyAIController::AEnemyAIController()
{
	PrimaryActorTick.bCanEverTick = true;
	// 플레이어 위치 추적은 매 프레임 불필요 — 0.1s 간격으로 충분
	PrimaryActorTick.TickInterval = 0.1f;
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	KHS_DEBUG(TEXT("[AI] OnPossess — %s → %s"), *GetName(), InPawn ? *InPawn->GetName() : TEXT("NULL"));
}

void AEnemyAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!UpdatePlayerInfo())
	{
		return;
	}

	// 거리 기반 CMC·Anim 틱 간격 조정 — AIC가 0.1s 간격으로 돌기 때문에 부하 없음
	APawn* ControlledPawn = GetPawn();
	if (ControlledPawn && CachedPlayerPawn.IsValid())
	{
		const float DistToPlayer = FVector::Dist(
			ControlledPawn->GetActorLocation(),
			CachedPlayerPawn->GetActorLocation()
		);
		
		AdjustPawnTickRates(ControlledPawn, DistToPlayer);
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

	// BT 결정은 0.2s 간격으로 충분 — 서비스·데코레이터 조건 평가 비용 절감
	if (UBrainComponent* Brain = GetBrainComponent())
	{
		Brain->SetComponentTickInterval(0.2f);
	}

	UBlackboardComponent* BB = GetBlackboardComponent();
	if (BB)
	{
		BB->SetValueAsBool(BBKey_bPlayerDead, false);
	}

	KHS_DEBUG(TEXT("%s — BehaviorTree 실행 완료."), *GetName());
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
	KHS_DEBUG(TEXT("%s — AI 중단."), *GetName());
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

void AEnemyAIController::AdjustPawnTickRates(APawn* ControlledPawn, float DistToPlayer)
{
	ACharacter* EnemyChar = Cast<ACharacter>(ControlledPawn);
	if (!EnemyChar)
	{
		return;
	}

	// 거리 구간별 틱 간격:
	//   근거리 (< NearThreshold) : 매 프레임 — 전투 중 이동·애니 끊김 방지
	//   중거리                   : CMC 30Hz / Anim 20Hz
	//   원거리 (> FarThreshold)  : CMC 20Hz / Anim 10Hz
	float NewCMCInterval;
	float NewAnimInterval;

	if (DistToPlayer < NearThreshold)
	{
		NewCMCInterval  = 0.0f;
		NewAnimInterval = 0.0f;
	}
	else if (DistToPlayer < FarThreshold)
	{
		NewCMCInterval  = MidCMCTickInterval;
		NewAnimInterval = MidAnimTickInterval;
	}
	else
	{
		NewCMCInterval  = FarCMCTickInterval;
		NewAnimInterval = FarAnimTickInterval;
	}

	if (UCharacterMovementComponent* MoveComp = EnemyChar->GetCharacterMovement())
	{
		MoveComp->SetComponentTickInterval(NewCMCInterval);
	}

	if (USkeletalMeshComponent* MeshComp = EnemyChar->GetMesh())
	{
		MeshComp->SetComponentTickInterval(NewAnimInterval);
	}
}
