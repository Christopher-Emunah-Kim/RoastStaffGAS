// Fill out your copyright notice in the Description page of Project Settings.

#include "Objects/GroundEffect/ChainTrapVortexActor.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "DrawDebugHelpers.h"
#include "RoastStaffGAS.h"
#include "GAS/Tags/RSGameplayTags.h"
#include "Subsystems/PoolingSubsystem.h"
#include "System/LoggingSystem.h"

AChainTrapVortexActor::AChainTrapVortexActor()
{
	PrimaryActorTick.bCanEverTick = false;
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
}

void AChainTrapVortexActor::BeginPlay()
{
	Super::BeginPlay();
	OnPoolDeactivate();
}

void AChainTrapVortexActor::OnPoolActivate()
{
	SetActorHiddenInGame(false);
}

void AChainTrapVortexActor::OnPoolDeactivate()
{
	SetActorHiddenInGame(true);

	GetWorld()->GetTimerManager().ClearTimer(SpawnFXDelayHandle);
	GetWorld()->GetTimerManager().ClearTimer(PullTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(DurationTimerHandle);

	PulledEnemies.Empty();

	CachedInstigatorASC = nullptr;
	CachedDamageGEClass = nullptr;
	CachedStunGEClass   = nullptr;
	CachedEffectRadius  = 0.f;
	CachedAmount        = 0.f;

	if (SpawnedFXComp && SpawnedFXComp->IsActive())
	{
		SpawnedFXComp->Deactivate();
		SpawnedFXComp = nullptr;
	}
}

void AChainTrapVortexActor::InitEffect(const FSkillEffectInitData& InitData)
{
	CachedInstigatorASC  = InitData.InstigatorASC;
	CachedDamageGEClass  = InitData.SkillGEClass;    // Instant 데미지 — GE_Hawkeye_ChainTrap_Damage
	CachedStunGEClass    = InitData.StatusGEClass;   // Duration Stun — GE_Hawkeye_ChainTrap_Stun
	CachedEffectRadius   = FMath::Max(1.f, InitData.EffectRadius);
	CachedAmount         = InitData.Amount;

	// 스폰 FX — Actor에 Attach해서 위치와 함께 움직임
	if (SpawnFX)
	{
		SpawnedFXComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
			SpawnFX, GetRootComponent(), NAME_None,
			FVector::ZeroVector, FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget, true);

		if (SpawnedFXComp)
		{
			SpawnedFXComp->SetVariableFloat(FName(TEXT("Radius")), CachedEffectRadius);
		}
	}

	// SpawnFX 재생 완료 후 수렴 시작 — FX 연출이 끝나는 시점에 효과 발생
	TWeakObjectPtr<AChainTrapVortexActor> WeakThis(this);
	const float SafeDuration = FMath::Max(InitData.Duration, 0.1f);

	GetWorld()->GetTimerManager().SetTimer(SpawnFXDelayHandle, [WeakThis, SafeDuration]()
	{
		if (!WeakThis.IsValid())
		{
			return;
		}

		// 수렴 틱 타이머
		WeakThis->GetWorld()->GetTimerManager().SetTimer(WeakThis->PullTimerHandle, [WeakThis]()
		{
			if (WeakThis.IsValid())
			{
				WeakThis->PullTick();
			}
		}, WeakThis->PullTickRate, true);

		// Duration 만료 타이머
		WeakThis->GetWorld()->GetTimerManager().SetTimer(WeakThis->DurationTimerHandle, [WeakThis]()
		{
			if (WeakThis.IsValid())
			{
				WeakThis->OnDurationExpired();
			}
		}, SafeDuration, false);

	}, SpawnFXDuration, false);

	KHS_INFO(TEXT("ChainTrap 초기화 — EffectRadius: %.0f | Duration: %.1fs"), CachedEffectRadius, SafeDuration);

}

void AChainTrapVortexActor::PullTick()
{
	const FVector Center = GetActorLocation();

	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = false;

	GetWorld()->OverlapMultiByChannel(
		Overlaps, Center, FQuat::Identity,
		ECC_Pawn, FCollisionShape::MakeSphere(CachedEffectRadius), QueryParams);

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

		// 첫 감지 시 목록 등록
		if (!PulledEnemies.Contains(Target))
		{
			PulledEnemies.Add(Target);
		}

		// XY 수렴 + Z는 중력 상쇄(bZOverride=true, Z속도=0)로 바닥 뚫림 방지
		const FVector ToCenter    = Center - Target->GetActorLocation();
		const float   CurrentDist = static_cast<float>(ToCenter.Size2D());
		const FVector PullDir     = ToCenter.GetSafeNormal2D();
		EnemyChar->LaunchCharacter(FVector(PullDir.X * CurrentDist * PullStrength,
			PullDir.Y * CurrentDist * PullStrength, 0.f), true, true);

	}
}

void AChainTrapVortexActor::OnDurationExpired()
{
	GetWorld()->GetTimerManager().ClearTimer(PullTimerHandle);

	// 수렴 완료 FX — 일회성, Actor 위치에 스폰 (색상 연출은 BurstFX Niagara에서 처리)
	if (BurstFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(	GetWorld(), BurstFX, GetActorLocation(),
			FRotator::ZeroRotator, FVector::OneVector,	true, true);
	}

	// 데미지 + 기절 GE Apply
	ApplyEffectsToTargets();
	
	ReturnToPool();
}

void AChainTrapVortexActor::ApplyEffectsToTargets()
{
	int32 HitCount = 0;
	for (AActor* Target : PulledEnemies)
	{
		if (!Target)
		{
			continue;
		}

		// Instant 데미지 먼저 — ExecCalc가 SetByCaller 읽음
		if (CachedDamageGEClass)
		{
			ApplyDamageGEToTarget(Target);
		}

		// Duration Stun — CC.Stun 태그 부여
		if (CachedStunGEClass)
		{
			ApplyGEToTarget(Target, CachedStunGEClass);
		}

		HitCount++;
	}

	KHS_INFO(TEXT("ChainTrap GE Apply — 대상: %d명 | 데미지: %.1f"), HitCount, CachedAmount);
}

void AChainTrapVortexActor::ApplyDamageGEToTarget(AActor* TargetActor)
{
	if (!TargetActor || !CachedInstigatorASC || !CachedDamageGEClass)
	{
		return;
	}

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (!TargetASC || !TargetASC->HasMatchingGameplayTag(RSTags::Team_Enemy))
	{
		return;
	}

	FGameplayEffectContextHandle Context = CachedInstigatorASC->MakeEffectContext();
	FGameplayEffectSpecHandle Spec = CachedInstigatorASC->MakeOutgoingSpec(CachedDamageGEClass, 1, Context);
	if (!Spec.IsValid())
	{
		return;
	}

	// RS_DamageExecCalc가 읽는 플레이어 기본 데미지 키
	Spec.Data->SetByCallerTagMagnitudes.Add(RSTags::Data_WeaponBaseDamage, CachedAmount);
	CachedInstigatorASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
}

void AChainTrapVortexActor::ApplyGEToTarget(AActor* TargetActor, TSubclassOf<UGameplayEffect> GEClass)
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
	FGameplayEffectSpecHandle Spec = CachedInstigatorASC->MakeOutgoingSpec(GEClass, 1, Context);
	if (!Spec.IsValid())
	{
		return;
	}

	CachedInstigatorASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
}

void AChainTrapVortexActor::ReturnToPool()
{
	PulledEnemies.Empty();

	GET_WORLD_SUBSYSTEM(UPoolingSubsystem, PoolSub)
	PoolSub->ReturnToPool(this);
}
