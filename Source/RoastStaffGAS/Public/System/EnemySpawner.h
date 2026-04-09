// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemySpawner.generated.h"

class AEnemyBaseCharacter;
class AEnemyProjectile;
class UBossHPBarWidget;

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
	/** 플레이어 위치 기준 랜덤 방향 외곽 스폰 위치 계산 */
	FVector CalculateOffScreenSpawnLocation(const FVector& PlayerLocation) const;
	/** AIType에 따라 타입별 확장 초기화 (ExtData 주입) */
	void InitializeEnemyByType(AEnemyBaseCharacter* Enemy, FName EnemyID);

	/** 보스 사망 시 호출 — HUD 해제 담당 */
	UFUNCTION()
	void OnBossKilled();

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
	/** 에너미 투사체 풀 예비 수량 (동시 사격 가능한 최대 에너미 수 * 여유분) */
	UPROPERTY(EditDefaultsOnly, Category = "MY|Spawn")
	int32 ProjectilePoolCount = 60;

	/** 투사체 풀에 등록할 AEnemyProjectile 파생 클래스 — BP에서 할당 */
	UPROPERTY(EditDefaultsOnly, Category = "MY|Spawn")
	TSubclassOf<AEnemyProjectile> EnemyProjectileClass;

	/** 현재 스폰된 보스의 HP Bar 위젯 — OnBossKilled 폴백 정리용 (UMS가 수명 보장) */
	TWeakObjectPtr<UBossHPBarWidget> CachedBossHPBar;
};
