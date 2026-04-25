// Fill out your copyright notice in the Description page of Project Settings.


#include "Objects/GroundEffect/PullVortexActor.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"

DECLARE_CYCLE_STAT(TEXT("PullVortex PullTick"), STAT_PullVortexTick, STATGROUP_Game);
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "RoastStaffGAS.h"
#include "GAS/Tags/RSGameplayTags.h"
#include "Subsystems/PoolingSubsystem.h"
#include "System/LoggingSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "DrawDebugHelpers.h"


APullVortexActor::APullVortexActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// 최소 루트 컴포넌트 — 트랜스폼 기준점
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
}

void APullVortexActor::BeginPlay()
{
	Super::BeginPlay();
	OnPoolDeactivate();
}

void APullVortexActor::OnPoolActivate()
{
	SetActorHiddenInGame(false);
}

void APullVortexActor::OnPoolDeactivate()
{
	SetActorHiddenInGame(true);

	GetWorld()->GetTimerManager().ClearTimer(PullTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(HitTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(DurationTimerHandle);

	CachedInstigatorASC = nullptr;
	CachedSkillGEClass  = nullptr;
	CachedStatusGEClass = nullptr;
	CachedAmount        = 0.f;
	CachedEffectRadius  = 0.f;
	RemainingHitCount   = 0;

	if (SpawnedFXComp && SpawnedFXComp->IsActive())
	{
		SpawnedFXComp->Deactivate();
		SpawnedFXComp = nullptr;
	}
}

void APullVortexActor::InitEffect(const FSkillEffectInitData& InitData)
{
	CachedInstigatorASC = InitData.InstigatorASC;
	CachedSkillGEClass  = InitData.SkillGEClass;
	CachedStatusGEClass = InitData.StatusGEClass;
	CachedAmount        = InitData.Amount;
	CachedEffectRadius  = FMath::Max(1.f, InitData.EffectRadius);
	RemainingHitCount   = FMath::Max(1, HitCount);

	// 흡입 타이머
	TWeakObjectPtr<APullVortexActor> WeakThis(this);
	GetWorld()->GetTimerManager().SetTimer(PullTimerHandle, [WeakThis]()
	{
		if (WeakThis.IsValid())
		{
			WeakThis->PullTick();
		}
	}, PullTickRate, true);

	// 데미지 히트 타이머
	GetWorld()->GetTimerManager().SetTimer(HitTimerHandle, [WeakThis]()
	{
		if (WeakThis.IsValid())
		{
			WeakThis->HitTick();
		}
	}, HitInterval, true);

	// Duration 후 풀 반납
	// 모든 HitTick이 완료된 이후에 반납되도록 HitCount * HitInterval보다 작지 않게 보정
	if (InitData.Duration > 0.f)
	{
		const float SafeDuration = FMath::Max(InitData.Duration, static_cast<float>(HitCount) * HitInterval + 0.1f);
		GetWorld()->GetTimerManager().SetTimer(DurationTimerHandle, [WeakThis]()
		{
			if (WeakThis.IsValid())
			{
				WeakThis->ReturnToPool();
			}
		}, SafeDuration, false);
	}

	// FX — Duration 동안 Actor에 Attach (Looping Niagara 권장)
	TRACE_BOOKMARK(TEXT("PullVortex_FX_SyncLoad"));
	if (UNiagaraSystem* FX = InitData.SkillFX.LoadSynchronous())
	{
		SpawnedFXComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
			FX, GetRootComponent(), NAME_None,
			FVector::ZeroVector, FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget, true);

		if (SpawnedFXComp)
		{
			SpawnedFXComp->SetVariableFloat(FName(TEXT("Radius")), CachedEffectRadius);
			SpawnedFXComp->SetVariableLinearColor(FName(TEXT("ElementColor")), InitData.ElementColor);
		}
	}

// #if WITH_EDITOR
// 	// PullRadius (파란색) + EffectRadius (빨간색) 시각화
// 	const float DbgDuration = InitData.Duration > 0.f ? InitData.Duration : 3.f;
// 	DrawDebugSphere(GetWorld(), GetActorLocation(), PullRadius,        24, FColor::Blue, false, DbgDuration);
// 	DrawDebugSphere(GetWorld(), GetActorLocation(), CachedEffectRadius, 24, FColor::Red,  false, DbgDuration);
// #endif

	KHS_DEBUG(TEXT("PullVortex 초기화 — EffectRadius: %.0f | PullRadius: %.0f | HitCount: %d | Duration: %.1fs | Amount: %.1f"),
		CachedEffectRadius, PullRadius, HitCount, InitData.Duration, CachedAmount);
}

void APullVortexActor::PullTick()
{
	SCOPE_CYCLE_COUNTER(STAT_PullVortexTick);
	const FVector Center = GetActorLocation();

	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams QueryParams;

	GetWorld()->OverlapMultiByChannel(Overlaps, Center, FQuat::Identity,
		ECC_Pawn, FCollisionShape::MakeSphere(PullRadius), QueryParams);

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Target = Overlap.GetActor();
		if (!Target)
		{
			continue;
		}

		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);
		if (!TargetASC || !TargetASC->HasMatchingGameplayTag(RSTags::Team_Enemy))
		{
			continue;
		}

		ACharacter* EnemyChar = Cast<ACharacter>(Target);
		if (!EnemyChar)
		{
			continue;
		}

		// 거리 비례 흡입 속도 — 멀수록 빠르게, 가까울수록 느리게
		// bXYOverride=true: 매 틱 XY 속도를 덮어써서 예측 가능한 이동
		const FVector ToCenter   = Center - Target->GetActorLocation();
		const float   CurrentDist = static_cast<float>(ToCenter.Size2D());
		const FVector PullDir     = ToCenter.GetSafeNormal2D();
		EnemyChar->LaunchCharacter(PullDir * CurrentDist * PullStrength, true, false);
	}
}

void APullVortexActor::HitTick()
{
	if (RemainingHitCount <= 0)
	{
		return;
	}

	const bool   bIsLastHit  = (RemainingHitCount == 1);
	const FVector Center     = GetActorLocation();
	const int32  CurrentHit  = HitCount - RemainingHitCount + 1;

	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams QueryParams;

	GetWorld()->OverlapMultiByChannel(Overlaps, Center, FQuat::Identity,
		ECC_Pawn, FCollisionShape::MakeSphere(CachedEffectRadius), QueryParams);

	// 컴포넌트 단위 감지 → 같은 액터 중복 처리 방지
	TSet<AActor*> ProcessedActors;
	int32 HitTargetCount = 0;

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Target = Overlap.GetActor();
		if (!Target || ProcessedActors.Contains(Target))
		{
			continue;
		}
		ProcessedActors.Add(Target);

		// 메인 데미지 GE
		if (CachedSkillGEClass)
		{
			ApplyGEToTarget(Target, CachedSkillGEClass);
			HitTargetCount++;
		}

		// 마지막 히트 — 바깥 방향 넉백 + StatusGE (넉다운)
		if (bIsLastHit)
		{
			ACharacter* EnemyChar = Cast<ACharacter>(Target);
			if (EnemyChar)
			{
				const FVector KnockDir = (Target->GetActorLocation() - Center).GetSafeNormal2D();
				EnemyChar->LaunchCharacter(KnockDir * KnockbackStrength, true, true);
			}

			if (CachedStatusGEClass)
			{
				ApplyGEToTarget(Target, CachedStatusGEClass);
			}
		}
	}

	KHS_INFO(TEXT("PullVortex 히트 [%d/%d] — 범위 내 적: %d | 마지막: %s"),
		CurrentHit, HitCount, HitTargetCount, bIsLastHit ? TEXT("YES") : TEXT("NO"));

	RemainingHitCount--;

	if (bIsLastHit)
	{
		GetWorld()->GetTimerManager().ClearTimer(HitTimerHandle);
	}
}

void APullVortexActor::ApplyGEToTarget(AActor* TargetActor, TSubclassOf<UGameplayEffect> GEClass)
{
	if (!TargetActor || !CachedInstigatorASC || !GEClass)
	{
		return;
	}

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (!TargetASC || !TargetASC->HasMatchingGameplayTag(RSTags::Team_Enemy))
	{
		return;
	}

	FGameplayEffectContextHandle Context = CachedInstigatorASC->MakeEffectContext();
	// MakeEffectContext가 세팅한 instigator를 유지 — GetInstigator()는 null(풀링 액터 미설정)이므로 덮어쓰지 않음
	FGameplayEffectSpecHandle Spec = CachedInstigatorASC->MakeOutgoingSpec(GEClass, 1, Context);
	if (!Spec.IsValid())
	{
		return;
	}

	Spec.Data->SetByCallerTagMagnitudes.Add(RSTags::Data_WeaponBaseDamage, CachedAmount);
	CachedInstigatorASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
}

void APullVortexActor::ReturnToPool()
{
	GET_WORLD_SUBSYSTEM(UPoolingSubsystem, PoolSub)
	PoolSub->ReturnToPool(this);
}
