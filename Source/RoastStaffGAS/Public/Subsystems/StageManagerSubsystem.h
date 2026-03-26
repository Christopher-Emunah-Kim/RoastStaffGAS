// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Data/DataTableStructs.h"
#include "StageManagerSubsystem.generated.h"

class AEnemySpawner;
class AEnemyBaseCharacter;

/**
 * UStageManagerSubsystem (WorldSubsystem)
 *
 * - 스테이지 진행, 웨이브 전환, 에너미 처치 집계를 담당한다.
 * - GDS를 통해 DT_Stage / DT_WaveData를 로드하고, AEnemySpawner에 스폰을 위임한다.
 * - ARSGameMode::BeginPlay → SetSpawner → StartStage 순서로 초기화한다.
 *
 * 접근: GetWorld()->GetSubsystem<UStageManagerSubsystem>()
 */
UCLASS()
class ROASTSTAFFGAS_API UStageManagerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** ARSGameMode::BeginPlay에서 Spawner 참조 주입 */
	void SetSpawner(AEnemySpawner* InSpawner);
	/** ARSGameMode::BeginPlay에서 호출 — GDS 조회 후 웨이브 타이머 시작 */
	void StartStage(FName StageID);
	/** AEnemySpawner::SpawnEnemy에서 스폰 직후 등록 */
	void RegisterAliveEnemy(AEnemyBaseCharacter* Enemy);
	/** AEnemyBaseCharacter::HandleDeath에서 ReturnToPool 전에 호출 */
	void UnregisterAliveEnemy(AEnemyBaseCharacter* Enemy);
	/** AEnemyBaseCharacter::OnEnemyKilledDel 구독 콜백 */
	UFUNCTION()
	void OnEnemyKilled(FName InEnemyID);

	FORCEINLINE int32 GetKillCount()  const { return KillCount; }
	FORCEINLINE int32 GetAliveCount() const { return AliveEnemies.Num(); }

private:
	/** SpawnInterval 루프 타이머 콜백 */
	void OnSpawnTimer();
	/** 웨이브 WaveIdx 활성화 — 스폰 타이머 재설정 */
	void ActivateWave(int32 WaveIdx);
	/** 현재 웨이브 SpawnEnemyIDs + SpawnWeights 기반 가중치 랜덤 선택 */
	FName SelectEnemyIDByWeight() const;

private:
	// 런타임 상태 
	/** 생존 중인 에너미 목록 — UPROPERTY로 GC 추적 */
	UPROPERTY()
	TArray<TObjectPtr<AEnemyBaseCharacter>> AliveEnemies;
	
	int32 KillCount        = 0;
	int32 CurrentWaveIndex = 0;

	/** 현재 웨이브 스폰 간격 반복 타이머 */
	FTimerHandle SpawnTimerHandle;
	/** 웨이브 전환 타이머 목록 — 인덱스가 CachedWaveData와 대응 */
	TArray<FTimerHandle> WaveTransitionTimers;

	// DT 캐시 
	FStageStaticData      CachedStageData;
	TArray<FWaveStaticData> CachedWaveData;  // WaveIndex 오름차순 정렬됨

	// 스포너 참조 (WeakPtr — Actor 수명에서 독립)
	TWeakObjectPtr<AEnemySpawner> Spawner;
};
