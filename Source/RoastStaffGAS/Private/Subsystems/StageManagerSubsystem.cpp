// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystems/StageManagerSubsystem.h"
#include "RoastStaffGAS.h"
#include "System/EnemySpawner.h"
#include "Character/Enemy/EnemyBaseCharacter.h"
#include "Subsystems/GameDataSubsystem.h"
#include "Subsystems/LevelUpSubsystem.h"
#include "Kismet/GameplayStatics.h"

// ─────────────────────────────────────────────────────────────────────────────
// Public API
// ─────────────────────────────────────────────────────────────────────────────

void UStageManagerSubsystem::SetSpawner(AEnemySpawner* InSpawner)
{
	if (!InSpawner)
	{
		KHS_WARN(TEXT("UStageManagerSubsystem::SetSpawner — InSpawner가 null입니다."));
		return;
	}
	Spawner = InSpawner;
}

void UStageManagerSubsystem::StartStage(FName StageID)
{
	// GDS 획득 (GameInstanceSubsystem — 항상 유효)
	GET_GI_SUBSYSTEM_FROM(UGameDataSubsystem, GDS, GetWorld()->GetGameInstance());

	// DT_Stage 조회
	if (!GDS->GetStageData(StageID, CachedStageData))
	{
		KHS_WARN(TEXT("UStageManagerSubsystem::StartStage — DT_Stage 조회 실패. StageID: %s"), *StageID.ToString());
		return;
	}

	// DT_WaveData 조회
	CachedWaveData = GDS->GetWaveDataByStage(StageID);
	if (CachedWaveData.IsEmpty())
	{
		KHS_WARN(TEXT("UStageManagerSubsystem::StartStage — DT_WaveData가 비어있음. StageID: %s"), *StageID.ToString());
		return;
	}

	// 스포너 풀 초기화 (DT_Stage의 스폰 가능 에너미 목록 기반)
	if (Spawner.IsValid())
	{
		Spawner->InitPools(CachedStageData.SpawnEnemyIDs);
	}
	else
	{
		KHS_WARN(TEXT("UStageManagerSubsystem::StartStage — Spawner가 유효하지 않아 InitPools 불가. SetSpawner 호출 여부 확인 필요."));
	}

	// 런타임 상태 초기화
	KillCount          = 0;
	CurrentWaveIndex   = 0;
	AliveEnemies.Empty();

	// 웨이브 전환 타이머 예약
	WaveTransitionTimers.SetNum(CachedWaveData.Num());

	for (int32 i = 0; i < CachedWaveData.Num(); ++i)
	{
		const FWaveStaticData& Wave = CachedWaveData[i];

		if (Wave.StartTime <= 0.f)
		{
			// Wave 0: 즉시 활성화
			ActivateWave(i);
		}
		else
		{
			// 이후 웨이브: StartTime 경과 후 전환
			FTimerDelegate Delegate;
			Delegate.BindUObject(this, &UStageManagerSubsystem::ActivateWave, i);
			GetWorld()->GetTimerManager().SetTimer(WaveTransitionTimers[i], Delegate, Wave.StartTime, false);
		}
	}

	KHS_INFO(TEXT("UStageManagerSubsystem — 스테이지 시작. StageID: %s / 웨이브 수: %d"),
		*StageID.ToString(), CachedWaveData.Num());
}

void UStageManagerSubsystem::RegisterAliveEnemy(AEnemyBaseCharacter* Enemy)
{
	if (!IsValid(Enemy))
	{
		KHS_WARN(TEXT("UStageManagerSubsystem::RegisterAliveEnemy — null Enemy"));
		return;
	}
	AliveEnemies.AddUnique(Enemy);
}

void UStageManagerSubsystem::UnregisterAliveEnemy(AEnemyBaseCharacter* Enemy)
{
	if (!IsValid(Enemy))
	{
		return;
	}
	AliveEnemies.Remove(Enemy);
}

void UStageManagerSubsystem::OnEnemyKilled(FName InEnemyID)
{
	KillCount++;
	KHS_INFO(TEXT("UStageManagerSubsystem::OnEnemyKilled — EnemyID: %s / 누적 처치: %d"),
		*InEnemyID.ToString(), KillCount);

	//레벨업 시스템에 적 사망 이벤트 전달.
	GET_GI_SUBSYSTEM_FROM(ULevelUpSubsystem, LevelUpSys, GetWorld()->GetGameInstance());
	LevelUpSys->OnEnemyKilled(InEnemyID);
}

// ─────────────────────────────────────────────────────────────────────────────
// Private
// ─────────────────────────────────────────────────────────────────────────────

void UStageManagerSubsystem::ActivateWave(int32 WaveIdx)
{
	if (!CachedWaveData.IsValidIndex(WaveIdx))
	{
		KHS_WARN(TEXT("UStageManagerSubsystem::ActivateWave — 유효하지 않은 WaveIdx: %d"), WaveIdx);
		return;
	}

	CurrentWaveIndex = WaveIdx;

	// 이전 스폰 타이머 정리 후 새 간격으로 재설정
	GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);

	const float Interval = CachedWaveData[WaveIdx].SpawnInterval;
	GetWorld()->GetTimerManager().SetTimer(
		SpawnTimerHandle,
		this,
		&UStageManagerSubsystem::OnSpawnTimer,
		Interval,
		true
	);

	KHS_INFO(TEXT("UStageManagerSubsystem::ActivateWave — Wave %d 활성화. SpawnInterval: %.1fs / MaxAlive: %d"),
		WaveIdx,
		CachedWaveData[WaveIdx].SpawnInterval,
		CachedWaveData[WaveIdx].MaxAliveCount);
}

void UStageManagerSubsystem::OnSpawnTimer()
{
	if (!CachedWaveData.IsValidIndex(CurrentWaveIndex))
	{
		return;
	}

	const FWaveStaticData& Wave = CachedWaveData[CurrentWaveIndex];

	// 최대 동시 생존 수 제한
	if (AliveEnemies.Num() >= Wave.MaxAliveCount)
	{
		return;
	}

	// 플레이어 Pawn 획득
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!PlayerPawn)
	{
		return;
	}

	// 스포너 유효성 검사
	if (!Spawner.IsValid())
	{
		KHS_WARN(TEXT("UStageManagerSubsystem::OnSpawnTimer — Spawner가 유효하지 않음"));
		return;
	}

	// 가중치 랜덤 EnemyID 선택
	const FName EnemyID = SelectEnemyIDByWeight();
	if (EnemyID.IsNone())
	{
		KHS_WARN(TEXT("UStageManagerSubsystem::OnSpawnTimer — SelectEnemyIDByWeight 실패"));
		return;
	}

	Spawner->SpawnEnemy(EnemyID, PlayerPawn->GetActorLocation());
}

FName UStageManagerSubsystem::SelectEnemyIDByWeight() const
{
	if (!CachedWaveData.IsValidIndex(CurrentWaveIndex))
	{
		return NAME_None;
	}

	const FWaveStaticData& Wave = CachedWaveData[CurrentWaveIndex];

	if (Wave.SpawnEnemyIDs.IsEmpty())
	{
		KHS_WARN(TEXT("UStageManagerSubsystem::SelectEnemyIDByWeight — SpawnEnemyIDs가 비어있음"));
		return NAME_None;
	}

	// 배열 길이 불일치 방어
	if (Wave.SpawnEnemyIDs.Num() != Wave.SpawnWeights.Num())
	{
		KHS_WARN(TEXT("UStageManagerSubsystem::SelectEnemyIDByWeight — SpawnEnemyIDs(%d)와 SpawnWeights(%d) 길이 불일치. 첫 번째 ID 반환"),
			Wave.SpawnEnemyIDs.Num(), Wave.SpawnWeights.Num());
		return Wave.SpawnEnemyIDs[0];
	}

	// 가중치 합산
	float TotalWeight = 0.f;
	for (const float W : Wave.SpawnWeights)
	{
		TotalWeight += W;
	}

	if (TotalWeight <= 0.f)
	{
		KHS_WARN(TEXT("UStageManagerSubsystem::SelectEnemyIDByWeight — 가중치 합산이 0. 첫 번째 ID 반환"));
		return Wave.SpawnEnemyIDs[0];
	}

	float Rand = FMath::FRandRange(0.f, TotalWeight);
	for (int32 i = 0; i < Wave.SpawnEnemyIDs.Num(); ++i)
	{
		Rand -= Wave.SpawnWeights[i];
		if (Rand <= 0.f)
		{
			return Wave.SpawnEnemyIDs[i];
		}
	}

	// 부동소수점 오차 fallback
	return Wave.SpawnEnemyIDs.Last();
}
