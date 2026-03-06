// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIController.generated.h"

/**
 * UEnemyAIController
 *
 * - 에너미 공통 AI 컨트롤러.
 * - BT 실행 및 Blackboard 키 갱신 담당.
 */

class UBehaviorTree;
class UBlackboardComponent;


UCLASS()
class ROASTSTAFFGAS_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	AEnemyAIController();

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaTime) override;
	
private:
	bool UpdatePlayerInfo();
	bool IsPlayerDead() const;
	
public:
	void StartAI(UBehaviorTree* BehaviorTree);

private:
	// tick 체크용 캐싱
	TWeakObjectPtr<APawn> CachedPlayerPawn;
	
public:
	// Blackboard 키 이름 상수
	static const FName BBKey_PlayerLocation;
	static const FName BBKey_bPlayerDead;
};
