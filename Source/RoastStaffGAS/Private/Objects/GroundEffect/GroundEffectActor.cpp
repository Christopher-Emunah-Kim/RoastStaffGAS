// Fill out your copyright notice in the Description page of Project Settings.


#include "Objects/GroundEffect/GroundEffectActor.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Components/SphereComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "RoastStaffGAS.h"
#include "GAS/Tags/RSGameplayTags.h"
#include "Subsystems/PoolingSubsystem.h"
#include "System/LoggingSystem.h"


AGroundEffectActor::AGroundEffectActor()
{
	PrimaryActorTick.bCanEverTick = false;

	OverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphere"));
	SetRootComponent(OverlapSphere);
	OverlapSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	OverlapSphere->SetCollisionObjectType(ECC_WorldDynamic);
	OverlapSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	OverlapSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void AGroundEffectActor::BeginPlay()
{
	Super::BeginPlay();
	OverlapSphere->OnComponentBeginOverlap.AddDynamic(this, &AGroundEffectActor::OnSphereBeginOverlap);
	OnPoolDeactivate();
}

void AGroundEffectActor::OnPoolActivate()
{
	// 충돌은 InitEffect 마지막에 활성화 — 캐시 세팅 전 Overlap 이벤트 방지
	SetActorHiddenInGame(false);
	SetActorEnableCollision(false);
	OverlapSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AGroundEffectActor::OnPoolDeactivate()
{
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	OverlapSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	GetWorld()->GetTimerManager().ClearTimer(DurationTimerHandle);

	if (SpawnedFXComp && SpawnedFXComp->IsActive())
	{
		SpawnedFXComp->Deactivate();
	}
	SpawnedFXComp = nullptr;
	CachedInstigatorASC  = nullptr;
	CachedOverlapGEClass = nullptr;
	CachedAmount         = 0.f;
}

void AGroundEffectActor::InitEffect(const FSkillEffectInitData& InitData)
{
	CachedInstigatorASC  = InitData.InstigatorASC;
	CachedOverlapGEClass = InitData.SkillGEClass;
	CachedAmount         = InitData.Amount;

	const float Radius = FMath::Max(1.f, InitData.EffectRadius);
	OverlapSphere->SetSphereRadius(Radius);

	// FX 스폰 후 Actor에 부착
	if (UNiagaraSystem* FX = InitData.SkillFX.LoadSynchronous())
	{
		SpawnedFXComp = UNiagaraFunctionLibrary::SpawnSystemAttached(FX, OverlapSphere, NAME_None,
			FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget, true);

		if (SpawnedFXComp)
		{
			SpawnedFXComp->SetVariableFloat(FName(TEXT("Radius")), Radius);
		}
	}

	if (InitData.Duration > 0.f)
	{
		TWeakObjectPtr<AGroundEffectActor> WeakThis(this);
		GetWorld()->GetTimerManager().SetTimer(DurationTimerHandle, [WeakThis]()
		{
			if (WeakThis.IsValid())
			{
				WeakThis->ReturnToPool();
			}
		}, InitData.Duration, false);
	}

	// 캐시 세팅 완료 후 충돌 활성화 — Overlap 이벤트가 캐시 null 상태에서 발생하지 않도록
	SetActorEnableCollision(true);
	OverlapSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	KHS_INFO(TEXT("GroundEffect 초기화 — Radius: %.0f | Duration: %.1fs | Amount: %.1f"),
		Radius, InitData.Duration, InitData.Amount);
}

void AGroundEffectActor::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComp,AActor* OtherActor,UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ApplyGEToTarget(OtherActor);
}

void AGroundEffectActor::ApplyGEToTarget(AActor* TargetActor)
{
	if (!TargetActor || !CachedInstigatorASC || !CachedOverlapGEClass)
	{
		KHS_WARN(TEXT("필수 데이터 누락."));
		return;
	}

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (!TargetASC || !TargetASC->HasMatchingGameplayTag(RSTags::Team_Enemy))
	{
		return;
	}

	FGameplayEffectContextHandle Context = CachedInstigatorASC->MakeEffectContext();
	Context.AddInstigator(GetInstigator(), GetInstigator());
	FGameplayEffectSpecHandle Spec = CachedInstigatorASC->MakeOutgoingSpec(CachedOverlapGEClass, 1, Context);
	if (!Spec.IsValid())
	{
		return;
	}

	Spec.Data->SetByCallerTagMagnitudes.Add(RSTags::Data_WeaponBaseDamage, CachedAmount);
	CachedInstigatorASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);

	KHS_INFO(TEXT("GroundEffect GE 적용 — Target: %s | Amount: %.1f"), *TargetActor->GetName(), CachedAmount);
}

void AGroundEffectActor::ReturnToPool()
{
	GET_WORLD_SUBSYSTEM(UPoolingSubsystem, PoolSub)
	PoolSub->ReturnToPool(this);
}
