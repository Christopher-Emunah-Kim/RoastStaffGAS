// Fill out your copyright notice in the Description page of Project Settings.


#include "Objects/GroundEffect/PullVortexActor.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "RoastStaffGAS.h"
#include "GAS/Tags/RSGameplayTags.h"
#include "Subsystems/PoolingSubsystem.h"
#include "System/LoggingSystem.h"


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
	if (InitData.Duration > 0.f)
	{
		GetWorld()->GetTimerManager().SetTimer(DurationTimerHandle, [WeakThis]()
		{
			if (WeakThis.IsValid())
			{
				WeakThis->ReturnToPool();
			}
		}, InitData.Duration, false);
	}

	KHS_INFO(TEXT("PullVortex 초기화 — EffectRadius: %.0f | PullRadius: %.0f | HitCount: %d | Duration: %.1fs"),
		CachedEffectRadius, PullRadius, HitCount, InitData.Duration);
}

void APullVortexActor::PullTick()
{
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

		const FVector PullDir = (Center - Target->GetActorLocation()).GetSafeNormal2D();
		// bXYOverride=false: 기존 수평 속도에 추가 / bZOverride=false: 수직 속도 유지
		EnemyChar->LaunchCharacter(PullDir * PullStrength, false, false);
	}
}

void APullVortexActor::HitTick()
{
	if (RemainingHitCount <= 0)
	{
		return;
	}

	const bool bIsLastHit = (RemainingHitCount == 1);
	const FVector Center  = GetActorLocation();

	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams QueryParams;
	
	GetWorld()->OverlapMultiByChannel(Overlaps, Center, FQuat::Identity,
		ECC_Pawn, FCollisionShape::MakeSphere(CachedEffectRadius), QueryParams);

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Target = Overlap.GetActor();
		if (!Target)
		{
			continue;
		}

		// 메인 데미지 GE
		if (CachedSkillGEClass)
		{
			ApplyGEToTarget(Target, CachedSkillGEClass);
		}

		// 마지막 히트 — 넉다운 등 StatusGE 추가 적용
		if (bIsLastHit && CachedStatusGEClass)
		{
			ApplyGEToTarget(Target, CachedStatusGEClass);
		}
	}

	RemainingHitCount--;

	if (bIsLastHit)
	{
		GetWorld()->GetTimerManager().ClearTimer(HitTimerHandle);
		KHS_INFO(TEXT("PullVortex 히트 완료 — 마지막 히트 처리"));
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
	Context.AddInstigator(GetInstigator(), GetInstigator());
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
