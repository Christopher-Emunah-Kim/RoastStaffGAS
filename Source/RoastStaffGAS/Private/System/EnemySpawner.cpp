// Fill out your copyright notice in the Description page of Project Settings.


#include "System/EnemySpawner.h"
#include "RoastStaffGAS.h"
#include "NavigationSystem.h"
#include "Character/Enemy/EnemyBaseCharacter.h"
#include "Character/Enemy/EnemyAIController.h"
#include "Character/Enemy/RangedEnemy.h"
#include "Character/Enemy/EliteEnemy.h"
#include "Character/Enemy/BossEnemy.h"
#include "Subsystems/PoolingSubsystem.h"
#include "Subsystems/StageManagerSubsystem.h"
#include "Subsystems/GameDataSubsystem.h"
#include "Subsystems/UIManagerSubsystem.h"
#include "UI/Enemy/BossHPBarWidget.h"
#include "Data/EnumUITypes.h"
#include "AbilitySystemInterface.h"

AEnemySpawner::AEnemySpawner()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AEnemySpawner::InitPools(const TArray<FName>& EnemyIDs)
{
	GET_GI_SUBSYSTEM_FROM(UGameDataSubsystem, GDS, GetWorld()->GetGameInstance())

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

	// 스폰 위치 계산 (NavMesh 유효 위치, 실패 시 ZeroVector)
	const FVector SpawnLocation = CalculateOffScreenSpawnLocation(PlayerLocation);
	if (SpawnLocation == FVector::ZeroVector)
	{
		KHS_WARN(TEXT("SpawnEnemy — NavMesh 유효 위치 탐색 실패. EnemyID: %s 스폰 스킵."), *EnemyID.ToString());
		return;
	}
	const FTransform SpawnTransform(FRotator::ZeroRotator, SpawnLocation);

	// 풀에서 꺼냄 (내부에서 OnPoolActivate 호출)
	GET_WORLD_SUBSYSTEM(UPoolingSubsystem, PoolSys)
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
	GET_WORLD_SUBSYSTEM(UStageManagerSubsystem, StageMgr)
	StageMgr->RegisterAliveEnemy(Enemy);

	// AddUniqueDynamic: 풀 재사용 시 중복 바인딩 방지
	Enemy->OnEnemyKilledDel.AddUniqueDynamic(StageMgr, &UStageManagerSubsystem::OnEnemyKilled);

	KHS_INFO(TEXT("SpawnEnemy — EnemyID: %s / 스폰 위치: %s"),*EnemyID.ToString(), *SpawnLocation.ToString());
}

void AEnemySpawner::InitializeEnemyByType(AEnemyBaseCharacter* Enemy, FName EnemyID)
{
	GET_GI_SUBSYSTEM_FROM(UGameDataSubsystem, GDS, GetWorld()->GetGameInstance())

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
	case EAIType::CHASE:
		// MeleeEnemy — InitializeEnemy 내부에서 처리
		break;

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

			// Boss HP Bar 열기 — UIManagerSettings에 BOSS_HP_BAR 매핑 필요 (에디터)
			GET_GI_SUBSYSTEM_FROM(UUIManagerSubsystem, UMS, GetWorld()->GetGameInstance())
			URSBaseWidget* BaseWidget = UMS->OpenUIByID(EUIID::BOSS_HP_BAR);
			UBossHPBarWidget* BossHPBar = Cast<UBossHPBarWidget>(BaseWidget);

			if (ensureMsgf(BossHPBar, TEXT("EnemySpawner: BOSS_HP_BAR Cast 실패. UIManagerSettings 매핑 확인 필요.")))
			{
				IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(Boss);
				UAbilitySystemComponent* BossASC = ASCInterface ? ASCInterface->GetAbilitySystemComponent() : nullptr;
				BossHPBar->BindToASC(BossASC, Boss->GetPhase2HPRatio());
				CachedBossHPBar = BossHPBar;
			}

			Boss->OnBossKilledDel.AddUniqueDynamic(this, &AEnemySpawner::OnBossKilled);
		}
		break;

	}
}

void AEnemySpawner::OnBossKilled()
{
	if (CachedBossHPBar.IsValid() && CachedBossHPBar->IsClosing())
	{
		// FadeOut 애니메이션 진행 중 — 완료 시 위젯이 UMS에 자체 정리 요청
		CachedBossHPBar = nullptr;
		return;
	}

	// FadeOut 없는 경우(Anim_FadeOut 미설정) — 즉시 정리
	GET_GI_SUBSYSTEM_FROM(UUIManagerSubsystem, UMS, GetWorld()->GetGameInstance())
	UMS->CloseUIByID(EUIID::BOSS_HP_BAR);
	CachedBossHPBar = nullptr;
}

FVector AEnemySpawner::CalculateOffScreenSpawnLocation(const FVector& PlayerLocation, int32 MaxAttempts) const
{
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavSys)
	{
		KHS_WARN(TEXT("CalculateOffScreenSpawnLocation — NavigationSystem 없음."));
		return FVector::ZeroVector;
	}

	// NavMesh 투영 탐색 범위: XY는 캡슐 반경 여유, Z는 플레이어 레벨 ±150만 허용
	const FVector QueryExtent(150.f, 150.f, 150.f);

	for (int32 i = 0; i < MaxAttempts; ++i)
	{
		const float AngleRad = FMath::DegreesToRadians(FMath::FRandRange(0.f, 360.f));
		const FVector Candidate = PlayerLocation + FVector(
			FMath::Cos(AngleRad) * OffScreenDistance,
			FMath::Sin(AngleRad) * OffScreenDistance,
			0.f
		);

		FNavLocation NavLocation;
		if (!NavSys->ProjectPointToNavigation(Candidate, NavLocation, QueryExtent))
		{
			continue;
		}

		// 플레이어와 Z 차이가 너무 크면 지하/공중 NavMesh 폴리곤 — 스킵
		if (FMath::Abs(NavLocation.Location.Z - PlayerLocation.Z) > 200.f)
		{
			continue;
		}

		// 라인 트레이스로 실제 바닥 지오메트리 확인 — NavMesh는 런타임 지오메트리를 보장하지 않음
		const FVector TraceStart(NavLocation.Location.X, NavLocation.Location.Y, NavLocation.Location.Z + 200.f);
		const FVector TraceEnd  (NavLocation.Location.X, NavLocation.Location.Y, NavLocation.Location.Z - 200.f);
		FHitResult HitResult;
		FCollisionQueryParams Params(SCENE_QUERY_STAT(EnemySpawnTrace), false);
		const bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, Params);
		if (!bHit)
		{
			continue;
		}

		// 바닥 표면 Z + 캡슐 절반 높이 → 캐릭터 루트를 바닥 위에 배치
		const FVector ProposedLocation(NavLocation.Location.X, NavLocation.Location.Y, HitResult.ImpactPoint.Z + 90.f);

		FVector AdjustedLocation = ProposedLocation;
		if (GetWorld()->FindTeleportSpot(this, AdjustedLocation, FRotator::ZeroRotator))
		{
			return AdjustedLocation;
		}

		continue;
	}

	KHS_WARN(TEXT("CalculateOffScreenSpawnLocation — %d회 시도 후 NavMesh 위치 탐색 실패."), MaxAttempts);
	return FVector::ZeroVector;
}
