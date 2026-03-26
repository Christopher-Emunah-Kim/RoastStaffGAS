// Fill out your copyright notice in the Description page of Project Settings.


#include "System/EnemySpawner.h"
#include "RoastStaffGAS.h"
#include "Character/Enemy/EnemyBaseCharacter.h"
#include "Character/Enemy/EnemyAIController.h"
#include "Subsystems/PoolingSubsystem.h"
#include "Subsystems/StageManagerSubsystem.h"
#include "Subsystems/GameDataSubsystem.h"

AEnemySpawner::AEnemySpawner()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AEnemySpawner::InitPools(const TArray<FName>& EnemyIDs)
{
	GET_WORLD_SUBSYSTEM(UPoolingSubsystem, PoolSys);
	GET_GI_SUBSYSTEM_FROM(UGameDataSubsystem, GDS, GetWorld()->GetGameInstance());

	ClassCache.Empty();

	for (const FName& EnemyID : EnemyIDs)
	{
		FEnemyStaticData EnemyData;
		if (!GDS->GetEnemyData(EnemyID, EnemyData))
		{
			KHS_WARN(TEXT("InitPools — EnemyID '%s' DT_Enemy 조회 실패. 건너뜀."), *EnemyID.ToString());
			continue;
		}

		if (EnemyData.EnemyClass.IsNull())
		{
			KHS_WARN(TEXT("InitPools — EnemyID '%s' EnemyClass가 비어있음. 건너뜀."), *EnemyID.ToString());
			continue;
		}

		TSubclassOf<AEnemyBaseCharacter> LoadedClass = EnemyData.EnemyClass.LoadSynchronous();
		if (!LoadedClass)
		{
			KHS_WARN(TEXT("InitPools — EnemyID '%s' EnemyClass 로드 실패. 건너뜀."), *EnemyID.ToString());
			continue;
		}

		ClassCache.Add(EnemyID, LoadedClass);
		PoolSys->InitializePool(LoadedClass, PoolCountPerClass);
	}

	KHS_INFO(TEXT("InitPools — %d개 에너미 클래스 풀 초기화 완료"), ClassCache.Num());
}

void AEnemySpawner::SpawnEnemy(FName EnemyID, const FVector& PlayerLocation)
{
	// 클래스 조회 (InitPools에서 로드된 캐시)
	TSubclassOf<AEnemyBaseCharacter>* ClassPtr = ClassCache.Find(EnemyID);
	if (!ClassPtr || !(*ClassPtr))
	{
		KHS_WARN(TEXT("SpawnEnemy — EnemyID '%s'에 대한 클래스가 캐시에 없음. InitPools 호출 여부 확인 필요."), *EnemyID.ToString());
		return;
	}

	// 스폰 위치 계산
	const FVector SpawnLocation = CalculateOffScreenSpawnLocation(PlayerLocation);
	const FTransform SpawnTransform(FRotator::ZeroRotator, SpawnLocation);

	// 풀에서 꺼냄 (내부에서 OnPoolActivate 호출)
	GET_WORLD_SUBSYSTEM(UPoolingSubsystem, PoolSys);
	AEnemyBaseCharacter* Enemy = PoolSys->SpawnPooledActor<AEnemyBaseCharacter>(*ClassPtr, SpawnTransform);
	if (!Enemy)
	{
		KHS_WARN(TEXT("SpawnEnemy — 풀 스폰 실패. EnemyID: %s"), *EnemyID.ToString());
		return;
	}

	// 에너미 초기화 (GDS 조회, AS 설정, BT 시작)
	Enemy->InitializeEnemy(EnemyID);

	// AI 초기 타겟 위치 주입 (BT 첫 틱부터 플레이어 방향으로 이동)
	if (AEnemyAIController* AIC = Cast<AEnemyAIController>(Enemy->GetController()))
	{
		AIC->SetInitialTargetLocation(PlayerLocation);
	}

	// StageManager 생존 목록 등록 및 처치 델리게이트 구독
	GET_WORLD_SUBSYSTEM(UStageManagerSubsystem, StageMgr);
	StageMgr->RegisterAliveEnemy(Enemy);

	// AddUniqueDynamic: 풀 재사용 시 중복 바인딩 방지
	Enemy->OnEnemyKilledDel.AddUniqueDynamic(StageMgr, &UStageManagerSubsystem::OnEnemyKilled);

	KHS_INFO(TEXT("SpawnEnemy — EnemyID: %s / 스폰 위치: %s"),*EnemyID.ToString(), *SpawnLocation.ToString());
}

FVector AEnemySpawner::CalculateOffScreenSpawnLocation(const FVector& PlayerLocation) const
{
	const float AngleDeg = FMath::FRandRange(0.f, 360.f);
	const float AngleRad = FMath::DegreesToRadians(AngleDeg);
	const FVector Offset(FMath::Cos(AngleRad) * OffScreenDistance,FMath::Sin(AngleRad),0.f	);
	
	return PlayerLocation + Offset;
}
