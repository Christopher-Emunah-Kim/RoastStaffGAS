// Fill out your copyright notice in the Description page of Project Settings.


#include "Objects/Projectile/BaseProjectile.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameplayEffect.h"
#include "RoastStaffGAS.h"
#include "GAS/Tags/RSGameplayTags.h"
#include "Subsystems/PoolingSubsystem.h"
#include "System/LoggingSystem.h"
#include "Data/EnumTypes.h"

ABaseProjectile::ABaseProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetSphereRadius(200.f);
	SphereComp->SetCollisionProfileName(TEXT("Projectile"));
	RootComponent = SphereComp;

	ProjectileComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileComp->InitialSpeed  = 1000.f;
	ProjectileComp->MaxSpeed      = 3000.f;
	ProjectileComp->bRotationFollowsVelocity = true;
	ProjectileComp->ProjectileGravityScale   = 0.f;
}

void ABaseProjectile::OnPoolActivate()
{
	ProjectileComp->SetUpdatedComponent(GetRootComponent());
	
	SetActorHiddenInGame(false);                                                                                 
	SetActorEnableCollision(true); 
}

void ABaseProjectile::OnPoolDeactivate()
{
	ProjectileComp->StopMovementImmediately();
	ProjectileComp->Velocity = FVector::ZeroVector;
	ProjectileComp->bIsHomingProjectile = false;
	ProjectileComp->HomingTargetComponent = nullptr;
	ProjectileComp->ProjectileGravityScale = 0.f;

	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	GetWorldTimerManager().ClearTimer(LifetimeTimerHandle);
}

void ABaseProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	SphereComp->OnComponentHit.AddDynamic(this, &ABaseProjectile::OnHit);
	OnPoolDeactivate();
}


void ABaseProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                            FVector NormalImpulse, const FHitResult& Hit)
{
	if (!OtherActor || OtherActor == GetOwner())
	{
		return;
	}

	// 폭발 공격 - 착탄 위치에서 범위 폭발 (벽/지형 포함 충돌대상 무관. AREA 타입)
	if (InitData.HitType == EHitType::AREA)
	{
		ExplodeArea(Hit.ImpactPoint);
		ReturnToPool();
		return;
	}

	// 일반 공격 처리
	HandleHitEvent(OtherActor, Hit);
}


void ABaseProjectile::ReturnToPool()
{
	GET_WORLD_SUBSYSTEM(UPoolingSubsystem, PoolSys);
	PoolSys->ReturnToPool(this);
}


void ABaseProjectile::ApplyMultipleEffectsToTarget(UAbilitySystemComponent* TargetASC, float DamageMultiplier)
{
	if (InitData.DamageGEClass)
	{
		ApplyEffectToTarget(TargetASC, InitData.DamageGEClass, InitData.Amount * DamageMultiplier);
	}

	if (InitData.StatusGEClass)
	{
		ApplyEffectToTarget(TargetASC, InitData.StatusGEClass, 0.f);
	}
}

void ABaseProjectile::ApplyEffectToTarget(UAbilitySystemComponent* TargetASC,  TSubclassOf<UGameplayEffect> EffectClass,
	float DamageValue)
{
	if (!TargetASC || !EffectClass || !InitData.InstigatorASC)
	{
		KHS_WARN(TEXT("유효하지 않은 파라미터"));
		return;
	}
	
	FGameplayEffectContextHandle Context = InitData.InstigatorASC->MakeEffectContext();
	Context.AddInstigator(GetInstigator(), this);

	FGameplayEffectSpecHandle Spec = InitData.InstigatorASC->MakeOutgoingSpec(EffectClass, 1.f, Context);

	if (!Spec.IsValid())
	{
		KHS_WARN(TEXT("GE Spec 생성 실패"));
		return;
	}

	//SetByCaller로 데미지 적용
	if (DamageValue > 0.f)
	{
		Spec.Data->SetByCallerTagMagnitudes.Add(RSTags::Data_Damage, -DamageValue);
	}

	InitData.InstigatorASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
}

void ABaseProjectile::OnProjectileInitialized()
{
	if (InitData.MoveType == EMoveType::HOMING)
	{
		if (InitData.HomingTarget.IsValid())
		{
			ProjectileComp->bIsHomingProjectile = true;
			ProjectileComp->HomingTargetComponent = InitData.HomingTarget.Get();
			ProjectileComp->HomingAccelerationMagnitude = InitData.TurnSpeed;
		}
		else
		{
			KHS_WARN(TEXT("HOMING 타겟 없음 — 직선 비행 폴백. SkillID: %s"), *InitData.SkillID.ToString());
		}
	}
	else if (InitData.MoveType == EMoveType::ARC)
	{
		// RotateAngleAxis로 짐벌락 없이 LaunchAngle만큼 상방 회전
		const FVector RightVec = FVector::CrossProduct(GetActorForwardVector(), FVector::UpVector).GetSafeNormal();
		const FVector LaunchVel = GetActorForwardVector().RotateAngleAxis(InitData.LaunchAngle, RightVec) * InitData.Speed;

		if (LaunchVel.IsNearlyZero())
		{
			KHS_WARN(TEXT("ARC LaunchAngle 계산 Zero — 에임 방향 폴백. SkillID: %s"), *InitData.SkillID.ToString());
		}
		else
		{
			ProjectileComp->Velocity = LaunchVel;
		}

		ProjectileComp->ProjectileGravityScale = InitData.GravityScale;
	}
}

void ABaseProjectile::OnProjectileExpired()
{
	// ARC 미착탄 수명 만료
	if (InitData.HitType == EHitType::AREA)
	{
		ExplodeArea(GetActorLocation());
	}
	// ReturnToPool은 OnLifetimeExpired()가 담당
}


void ABaseProjectile::HandleHitEvent(AActor* OtherActor, const FHitResult& Hit)
{
	// Enemy ASC 보유 오브젝트에만 반응
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
	if (!TargetASC) //GAS오브젝트가 아님.
	{
		return;
	}
	
	// 자식 충돌 처리 — true 반환 시 베이스가 GE처리하고 ReturnPool
	const bool bHasEnemyTag = TargetASC->HasMatchingGameplayTag(RSTags::Team_Enemy);
	const bool bShouldDestroy = OnProjectileHit(OtherActor, Hit);

	if (bHasEnemyTag && bShouldDestroy)
	{
		ApplyMultipleEffectsToTarget(TargetASC, 1.f);
		ReturnToPool();
	}
}

void ABaseProjectile::ExplodeArea(const FVector& Center)
{
	if (bHasExploded)
	{
		return;
	}
	bHasExploded = true;
	
	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(GetInstigator());

	GetWorld()->OverlapMultiByChannel(Overlaps, Center,FQuat::Identity,ECC_Pawn,
		FCollisionShape::MakeSphere(InitData.HitRadius), QueryParams
	);

	TSet<AActor*> HitActors;
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* OverlappedActor = Overlap.GetActor();
		if (!OverlappedActor || HitActors.Contains(OverlappedActor))
		{
			continue;
		}

		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OverlappedActor);
		if (!TargetASC || !TargetASC->HasMatchingGameplayTag(RSTags::Team_Enemy))
		{
			continue;
		}

		HitActors.Add(OverlappedActor); //한번 때린 적은 무시

		// 거리 기반 데미지 감쇠율 (Area 판정 규칙)
		const float Dist = FVector::Dist(Center, OverlappedActor->GetActorLocation());
		const float Ratio = (InitData.HitRadius > 0.f) ? (Dist / InitData.HitRadius) : 0.f;

		float DamageMultiplier = 1.f;
		if (Ratio >= 0.7f)
		{
			DamageMultiplier = 0.4f;
		}
		else if (Ratio >= 0.3f)
		{
			DamageMultiplier = 0.7f;
		}

		ApplyMultipleEffectsToTarget(TargetASC, DamageMultiplier);
	}
	
	
}

void ABaseProjectile::OnLifetimeExpired()
{
	OnProjectileExpired();
	GET_WORLD_SUBSYSTEM(UPoolingSubsystem, PoolSys);
	PoolSys->ReturnToPool(this);
}

void ABaseProjectile::InitProjectile(const FProjectileInitData& InInitData)
{
	// 풀 재사용 대비 상태 리셋
	bHasExploded = false;

	// 투사체 기본 초기화
	InitData = InInitData;

	// 속도 세팅
	ProjectileComp->InitialSpeed = InitData.Speed;
	ProjectileComp->MaxSpeed     = InitData.Speed;
	ProjectileComp->Velocity     = GetActorForwardVector() * InitData.Speed;

	// 수명 타이머
	const float SafeLifetime = InitData.Lifetime > 0.f ? InitData.Lifetime : 5.f;
	if (InitData.Lifetime <= 0.f)
	{
		KHS_WARN(TEXT("Lifetime이 0 이하 — 기본값 5.f 적용"));
	}

	GetWorldTimerManager().SetTimer(LifetimeTimerHandle,	this,
		&ABaseProjectile::OnLifetimeExpired,SafeLifetime,false);

	// 자식 타입별 추가 초기화
	OnProjectileInitialized();
}

