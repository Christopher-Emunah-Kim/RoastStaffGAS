// Fill out your copyright notice in the Description page of Project Settings.

#include "Objects/AutomatonActor.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "RoastStaffGAS.h"
#include "GAS/Tags/RSGameplayTags.h"
#include "Subsystems/PoolingSubsystem.h"
#include "System/LoggingSystem.h"
#include "Data/RuntimeDataStructs.h"
#include "Objects/Projectile/BaseProjectile.h"
#include "Math/UnrealMathUtility.h"

AAutomatonActor::AAutomatonActor()
{
	PrimaryActorTick.bCanEverTick = false;
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
}

void AAutomatonActor::BeginPlay()
{
	Super::BeginPlay();
	OnPoolDeactivate();
}

void AAutomatonActor::OnPoolActivate()
{
	SetActorHiddenInGame(false);
}

void AAutomatonActor::OnPoolDeactivate()
{
	SetActorHiddenInGame(true);

	GetWorld()->GetTimerManager().ClearTimer(TargetUpdateHandle);
	GetWorld()->GetTimerManager().ClearTimer(FireTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(HealTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(LifetimeTimerHandle);

	InstigatorASC       = nullptr;
	HealGEClass         = nullptr;
	CachedDamageGEClass = nullptr;
	CachedHealAmount    = 0.f;
	CachedDamageAmount  = 0.f;
	CachedLifetime   = 0.f;
	CachedProjSpeed  = 1200.f;
	CachedSpawnCount = SpreadCount;
	CachedForwardDir = FVector::ForwardVector;
	bNoTargetWarned  = false;

	if (SpawnedFXComp && SpawnedFXComp->IsActive())
	{
		SpawnedFXComp->Deactivate();
		SpawnedFXComp = nullptr;
	}
}

void AAutomatonActor::InitEffect(const FSkillEffectInitData& InitData)
{
	if (!InitData.InstigatorASC)
	{
		KHS_ERROR(TEXT("AutomatonActor::InitEffect — InstigatorASC가 null. 즉시 ReturnToPool."));
		ReturnToPool();
		return;
	}

	InstigatorASC        = InitData.InstigatorASC;
	HealGEClass          = InitData.SkillGEClass;     // 힐 GE — GE_Hawkeye_AutomatonHeal
	CachedDamageGEClass  = InitData.StatusGEClass;    // 투사체 데미지 GE — DT StatusGEClass
	CachedHealAmount     = InitData.Amount;
	CachedDamageAmount   = InitData.Amount * ProjectileDamageMultiplier;
	CachedLifetime   = FMath::Max(InitData.Duration, 0.1f);
	CachedSpawnCount = FMath::Max(1, InitData.SpawnCount);
	CachedForwardDir = InitData.InstigatorForward.GetSafeNormal();
	if (CachedForwardDir.IsNearlyZero())
	{
		CachedForwardDir = FVector::ForwardVector;
	}

	// 스폰 FX
	if (SpawnFX)
	{
		SpawnedFXComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
			SpawnFX, GetRootComponent(), NAME_None,
			FVector::ZeroVector, FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget, true);
	}

	// 초기 방향 — 스폰 즉시 적 탐색 후 세팅
	TargetUpdateTick();

	TWeakObjectPtr<AAutomatonActor> WeakThis(this);

	// 타겟 방향 주기 갱신
	GetWorld()->GetTimerManager().SetTimer(TargetUpdateHandle, [WeakThis]()
	{
		if (WeakThis.IsValid())
		{
			WeakThis->TargetUpdateTick();
		}
	}, TargetUpdateInterval, true);

	// 투사체 발사
	GetWorld()->GetTimerManager().SetTimer(FireTimerHandle, [WeakThis]()
	{
		if (WeakThis.IsValid())
		{
			WeakThis->FireTick();
		}
	}, AutomatonFireInterval, true);

	// 힐
	GetWorld()->GetTimerManager().SetTimer(HealTimerHandle, [WeakThis]()
	{
		if (WeakThis.IsValid())
		{
			WeakThis->HealTick();
		}
	}, AutomatonHealInterval, true);

	// 수명 타이머
	GetWorld()->GetTimerManager().SetTimer(LifetimeTimerHandle, [WeakThis]()
	{
		if (WeakThis.IsValid())
		{
			WeakThis->OnLifetimeExpired();
		}
	}, CachedLifetime, false);

	KHS_INFO(TEXT("Automaton 초기화 — Lifetime: %.1fs | SpawnCount: %d"), CachedLifetime, CachedSpawnCount);
}

void AAutomatonActor::TargetUpdateTick()
{
	const FVector Origin = GetActorLocation();

	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = false;

	GetWorld()->OverlapMultiByChannel(
		Overlaps, Origin, FQuat::Identity,
		ECC_Pawn, FCollisionShape::MakeSphere(TargetSearchRadius), QueryParams);

	float   MinDistSq    = FLT_MAX;
	AActor* NearestEnemy = nullptr;

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Candidate = Overlap.GetActor();
		if (!Candidate)
		{
			continue;
		}

		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Candidate);
		if (!TargetASC || !TargetASC->HasMatchingGameplayTag(RSTags::Team_Enemy))
		{
			continue;
		}

		const float DistSq = FVector::DistSquared(Origin, Candidate->GetActorLocation());
		if (DistSq < MinDistSq)
		{
			MinDistSq    = DistSq;
			NearestEnemy = Candidate;
		}
	}

	if (NearestEnemy)
	{
		// 적 발견 → 해당 방향으로 회전, 경고 플래그 리셋
		const FVector ToTarget = (NearestEnemy->GetActorLocation() - Origin).GetSafeNormal2D();
		if (!ToTarget.IsNearlyZero())
		{
			SetActorRotation(ToTarget.ToOrientationRotator());
		}
		bNoTargetWarned = false;
	}
	else
	{
		// 범위 내 적 없음 — 폴백 방향 유지, 경고 1회만
		if (!bNoTargetWarned)
		{
			KHS_WARN(TEXT("Automaton: 탐색 범위(%.0f) 내 적 없음 — CachedForward 방향 유지"), TargetSearchRadius);
			SetActorRotation(CachedForwardDir.ToOrientationRotator());
			bNoTargetWarned = true;
		}
	}
}

void AAutomatonActor::FireTick()
{
	if (!ProjectileClass)
	{
		KHS_WARN(TEXT("Automaton: ProjectileClass 미설정 — 발사 스킵"));
		return;
	}

	GET_WORLD_SUBSYSTEM(UPoolingSubsystem, PoolSub)

	const FVector Forward = GetActorForwardVector();
	const FVector Origin  = GetActorLocation();

	// SpawnCount 개수만큼 Spread 발사 (SpreadCount와 다르면 앞에서부터 사용)
	const int32 FireCount = FMath::Min(CachedSpawnCount, SpreadCount);
	for (int32 i = 0; i < FireCount; ++i)
	{
		// Forward 기준 좌우 균등 분산 — 각도를 쿼터니언으로 회전
		const FQuat   YawQuat  = FQuat(FVector::UpVector, FMath::DegreesToRadians(SpreadAngles[i]));
		const FVector FireDir  = YawQuat.RotateVector(Forward);
		const FVector SpawnLoc = Origin + FireDir * ProjectileSpawnOffset;
		const FRotator SpawnRot = FireDir.ToOrientationRotator();

		ABaseProjectile* Projectile = PoolSub->SpawnPooledActor<ABaseProjectile>(
			ProjectileClass, FTransform(SpawnRot, SpawnLoc));

		if (!Projectile)
		{
			KHS_WARN(TEXT("Automaton: 투사체 스폰 실패 (인덱스 %d)"), i);
			continue;
		}

		FProjectileInitData ProjInitData;
		ProjInitData.InstigatorASC = InstigatorASC;
		ProjInitData.DamageGEClass = CachedDamageGEClass;  // StatusGEClass → 투사체 데미지 GE
		ProjInitData.Amount        = CachedDamageAmount;
		ProjInitData.Speed         = CachedProjSpeed;
		ProjInitData.Lifetime      = TargetSearchRadius / FMath::Max(1.f, CachedProjSpeed);
		ProjInitData.MoveType      = EMoveType::LINEAR;
		ProjInitData.HitType       = EHitType::SINGLE;
		ProjInitData.SpawnCount    = 1;

		Projectile->InitProjectile(ProjInitData);
	}
}

void AAutomatonActor::HealTick()
{
	if (!InstigatorASC || !HealGEClass)
	{
		KHS_WARN(TEXT("Automaton: 힐 스킵 — InstigatorASC(%s) HealGEClass(%s)"),
			InstigatorASC ? TEXT("OK") : TEXT("null"),
			HealGEClass   ? TEXT("OK") : TEXT("null"));
		return;
	}

	FGameplayEffectContextHandle Context = InstigatorASC->MakeEffectContext();
	FGameplayEffectSpecHandle Spec = InstigatorASC->MakeOutgoingSpec(HealGEClass, 1, Context);
	if (!Spec.IsValid())
	{
		return;
	}

	Spec.Data->SetByCallerTagMagnitudes.Add(RSTags::Data_WeaponBaseDamage, CachedHealAmount);
	InstigatorASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());

	KHS_INFO(TEXT("Automaton 힐 적용 — 힐량: %.1f"), CachedHealAmount);
}

void AAutomatonActor::OnLifetimeExpired()
{
	KHS_INFO(TEXT("Automaton 수명 만료 — ReturnToPool"));
	ReturnToPool();
}

void AAutomatonActor::ReturnToPool()
{
	GET_WORLD_SUBSYSTEM(UPoolingSubsystem, PoolSub)
	PoolSub->ReturnToPool(this);
}
