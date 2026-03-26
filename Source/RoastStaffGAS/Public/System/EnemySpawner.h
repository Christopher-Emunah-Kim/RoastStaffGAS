// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemySpawner.generated.h"

class AEnemyBaseCharacter;

/**
 * AEnemySpawner
 *
 * - 레벨에 배치하는 에너미 스폰 담당 Actor.
 * - UStageManagerSubsystem::StartStage → InitPools(EnemyIDs) : DT_Enemy 기반 클래스 풀 초기화
 * - UStageManagerSubsystem의 SpawnEnemy 요청을 받아 화면 외곽에 에너미를 스폰한다.
 * - 스폰된 에너미에 InitializeEnemy, SetInitialTargetLocation, RegisterAliveEnemy를 순서대로 호출한다.
 */
UCLASS()
class ROASTSTAFFGAS_API AEnemySpawner : public AActor
{
	GENERATED_BODY()

public:
	AEnemySpawner();

	/** UStageManagerSubsystem::StartStage에서 호출 — EnemyIDs 목록 기반 DT_Enemy 조회 후 풀 초기화 */
	void InitPools(const TArray<FName>& EnemyIDs);
	/** UStageManagerSubsystem이 호출 — EnemyID 기반 스폰 실행 */
	void SpawnEnemy(FName EnemyID, const FVector& PlayerLocation);

private:
	/** 플레이어 위치 기준 랜덤 방향 외곽 스폰 위치 계산 */
	FVector CalculateOffScreenSpawnLocation(const FVector& PlayerLocation) const;

private:
	/** EnemyID → 에너미 클래스 런타임 캐시 (DT_Enemy.EnemyClass 로드 결과) */
	UPROPERTY()
	TMap<FName, TSubclassOf<AEnemyBaseCharacter>> ClassCache;
	/** 플레이어로부터의 스폰 반경 (언리얼 유닛) */
	UPROPERTY(EditDefaultsOnly, Category = "MY|Spawn")
	float OffScreenDistance = 1500.f;
	/** 클래스당 풀 예비 수량 */
	UPROPERTY(EditDefaultsOnly, Category = "MY|Spawn")
	int32 PoolCountPerClass = 30;
};
