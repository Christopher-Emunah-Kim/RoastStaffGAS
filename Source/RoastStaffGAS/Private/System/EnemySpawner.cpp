// Fill out your copyright notice in the Description page of Project Settings.


#include "System/EnemySpawner.h"
#include "RoastStaffGAS.h"
#include "Character/Enemy/EnemyBaseCharacter.h"
#include "Character/Enemy/EnemyAIController.h"
#include "Character/Enemy/RangedEnemy.h"
#include "Character/Enemy/EliteEnemy.h"
#include "Character/Enemy/BossEnemy.h"
#include "Objects/Projectile/EnemyProjectile.h"
#include "Subsystems/PoolingSubsystem.h"
#include "Subsystems/StageManagerSubsystem.h"
#include "Subsystems/GameDataSubsystem.h"

AEnemySpawner::AEnemySpawner()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AEnemySpawner::InitPools(const TArray<FName>& EnemyIDs)
{
	GET_GI_SUBSYSTEM_FROM(UGameDataSubsystem, GDS, GetWorld()->GetGameInstance());

	ClassCache.Empty();

	for (const FName& EnemyID : EnemyIDs)
	{
		FEnemyStaticData EnemyData;
		if (!GDS->GetEnemyData(EnemyID, EnemyData))
		{
			KHS_WARN(TEXT("EnemyID '%s' DT_Enemy 조회 실패. 건너뜀."), *EnemyID.ToString());
			continue;
		}

		if (EnemyData.EnemyClass.IsNull())
		{
			KHS_WARN(TEXT("EnemyID '%s' EnemyClass가 비어있음. 건너뜀."), *EnemyID.ToString());
			continue;
		}

		TSubclassOf<AEnemyBaseCharacter> LoadedClass = EnemyData.EnemyClass.LoadSynchronous();
		if (!LoadedClass)
		{
			KHS_WARN(TEXT("EnemyID '%s' EnemyClass 로드 실패. 건너뜀."), *EnemyID.ToString());
			continue;
		}

		ClassCache.Add(EnemyID, LoadedClass);
	}

	KHS_INFO(TEXT("InitPools — ClassCache %d개 빌드 완료 (풀 초기화는 PoolingSubsystem 위임)"), ClassCache.Num());
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

	// AIType별 확장 초기화 (원거리·엘리트·보스는 ExtData 추가 주입)
	InitializeEnemyByType(Enemy, EnemyID);

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

void AEnemySpawner::InitializeEnemyByType(AEnemyBaseCharacter* Enemy, FName EnemyID) const
{
	GET_GI_SUBSYSTEM_FROM(UGameDataSubsystem, GDS, GetWorld()->GetGameInstance());

	FEnemyStaticData StaticData;
	if (!GDS->GetEnemyData(EnemyID, StaticData))
	{
		KHS_WARN(TEXT("InitializeEnemyByType — EnemyID '%s' StaticData 조회 실패."), *EnemyID.ToString());
		return;
	}

	// CHASE 타입은 MeleeEnemy — InitializeMeleeParams는 EnemyBaseCharacter::InitializeEnemy 내부에서 처리
	if (StaticData.AIType == EAIType::CHASE)
	{
		return;
	}

	// RANGED / ELITE / BOSS 타입은 FEnemyExtData 추가 주입
	FEnemyExtData ExtData;
	if (!GDS->GetEnemyExtData(EnemyID, ExtData))
	{
		KHS_WARN(TEXT("EnemyID '%s' ExtData 조회 실패. 파라미터 주입 스킵."), *EnemyID.ToString());
		return;
	}

	switch (StaticData.AIType)
	{
	case EAIType::RANGED:
		if (ARangedEnemy* Ranged = Cast<ARangedEnemy>(Enemy))
		{
			Ranged->InitializeRangedParams(StaticData.AttackDamage, ExtData);
		}
		break;

	case EAIType::ELITE:
		if (AEliteEnemy* Elite = Cast<AEliteEnemy>(Enemy))
		{
			Elite->InitializeEliteParams(StaticData.AttackDamage, ExtData);
		}
		break;

	case EAIType::BOSS:
		if (ABossEnemy* Boss = Cast<ABossEnemy>(Enemy))
		{
			Boss->InitializeBossParams(StaticData.AttackDamage, ExtData);
			// TODO: Boss HUD 등록 — WBP_BossHPBar 구현 후 UIManager 연동
			Boss->OnBossKilledDel.AddUniqueDynamic(this, &AEnemySpawner::OnBossKilled);
		}
		break;

	default:
		break;
	}
}

void AEnemySpawner::OnBossKilled()
{
	// TODO: UIManager를 통해 WBP_BossHPBar 숨기기
	KHS_INFO(TEXT("EnemySpawner — 보스 처치. HUD 해제 대기 중 (WBP_BossHPBar 미구현)."));
}

FVector AEnemySpawner::CalculateOffScreenSpawnLocation(const FVector& PlayerLocation) const
{
	const float AngleDeg = FMath::FRandRange(0.f, 360.f);
	const float AngleRad = FMath::DegreesToRadians(AngleDeg);
	const FVector Offset(FMath::Cos(AngleRad) * OffScreenDistance,FMath::Sin(AngleRad),0.f	);
	
	return PlayerLocation + Offset;
}
