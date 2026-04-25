// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemySpawner.generated.h"

class AEnemyBaseCharacter;
class AEnemyProjectile;
class URSHUDWidget;
class UBehaviorTree;

/**
 * AEnemySpawner
 *
 * - 레벨에 배치하는 에너미 스폰 담당 Actor.
 */
UCLASS()
class ROASTSTAFFGAS_API AEnemySpawner : public AActor
{
	GENERATED_BODY()

public:
	AEnemySpawner();

	/** UStageManagerSubsystem::StartStage에서 호출 — EnemyIDs 목록 기반 DT_Enemy 조회 후 ClassCache 빌드 */
	void InitPools(const TArray<FName>& EnemyIDs);
	/** UStageManagerSubsystem이 호출 — EnemyID 기반 스폰 실행 */
	void SpawnEnemy(FName EnemyID, const FVector& PlayerLocation);

	/** RSGameMode::BuildPreWarmList에서 풀 요청 구성용 */
	TSubclassOf<AEnemyProjectile> GetEnemyProjectileClass() const { return EnemyProjectileClass; }
	int32 GetProjectilePoolCount() const { return ProjectilePoolCount; }
	int32 GetPoolCountPerClass() const { return PoolCountPerClass; }

private:
	/**
	 * 플레이어 위치 기준 랜덤 방향 외곽 NavMesh 유효 위치 계산.
	 * MaxAttempts 횟수까지 NavMesh 투영을 시도하고, 전부 실패 시 FVector::ZeroVector 반환.
	 */
	FVector CalculateOffScreenSpawnLocation(const FVector& PlayerLocation, int32 MaxAttempts = 5) const;
	/** AIType에 따라 타입별 확장 초기화 (ExtData 주입) */
	void InitializeEnemyByType(AEnemyBaseCharacter* Enemy, FName EnemyID);

	/** 보스 사망 시 호출 — HUD 해제 담당 */
	UFUNCTION()
	void OnBossKilled();

private:
	/** EnemyID → 에너미 클래스 런타임 캐시 (DT_Enemy.EnemyClass 로드 결과) */
	UPROPERTY()
	TMap<FName, TSubclassOf<AEnemyBaseCharacter>> ClassCache;
	/** EnemyID → BehaviorTree 강참조 캐시 — GC 방지용. StartEnemyAI LoadSynchronous를 FindObject 경로로 처리 */
	UPROPERTY()
	TMap<FName, TObjectPtr<UBehaviorTree>> BTCache;
	/** 플레이어로부터의 스폰 반경 (언리얼 유닛) */
	UPROPERTY(EditDefaultsOnly, Category = "MY|Spawn")
	float OffScreenDistance = 1500.f;
	
	/** 클래스당 풀 예비 수량 — GC 스파이크 방지용. 최대 동시 활성 적 수 기준으로 설정 */
	UPROPERTY(EditDefaultsOnly, Category = "MY|Spawn")
	int32 PoolCountPerClass = 60;
	/** 에너미 투사체 풀 예비 수량 (동시 사격 가능한 최대 에너미 수 * 여유분) */
	UPROPERTY(EditDefaultsOnly, Category = "MY|Spawn")
	int32 ProjectilePoolCount = 60;

	/** 투사체 풀에 등록할 AEnemyProjectile 파생 클래스 — BP에서 할당 */
	UPROPERTY(EditDefaultsOnly, Category = "MY|Spawn")
	TSubclassOf<AEnemyProjectile> EnemyProjectileClass;

	/** 보스 사망 폴백용 HUD 위젯 약참조 — HideBossHPBar 호출에 사용 */
	TWeakObjectPtr<URSHUDWidget> CachedHUDWidget;
};
