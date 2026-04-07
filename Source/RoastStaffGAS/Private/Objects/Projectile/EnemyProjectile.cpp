// Fill out your copyright notice in the Description page of Project Settings.

#include "Objects/Projectile/EnemyProjectile.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffectTypes.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GAS/Tags/RSGameplayTags.h"
#include "Subsystems/PoolingSubsystem.h"
#include "RoastStaffGAS.h"

AEnemyProjectile::AEnemyProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetSphereRadius(30.f);
	// 벽만 Block, Pawn은 Overlap — 에너미끼리 투사체 미차단
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SphereComp->SetCollisionObjectType(ECC_GameTraceChannel1);  // 커스텀 "EnemyProjectile" 채널
	SphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComp->SetCollisionResponseToChannel(ECC_WorldStatic,  ECR_Block);
	SphereComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	SphereComp->SetCollisionResponseToChannel(ECC_Pawn,         ECR_Overlap);
	RootComponent = SphereComp;

	ProjectileComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileComp->InitialSpeed = 600.f;
	ProjectileComp->MaxSpeed     = 3000.f;
	ProjectileComp->bRotationFollowsVelocity = true;
	ProjectileComp->ProjectileGravityScale   = 0.f;
}

// ─────────────────────────────────────────────────────────────────────────────
// IPoolableInterface
// ─────────────────────────────────────────────────────────────────────────────

void AEnemyProjectile::OnPoolActivate()
{
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	ProjectileComp->SetUpdatedComponent(GetRootComponent());
}

void AEnemyProjectile::OnPoolDeactivate()
{
	ProjectileComp->StopMovementImmediately();
	ProjectileComp->Velocity = FVector::ZeroVector;

	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);

	GetWorldTimerManager().ClearTimer(LifetimeTimerHandle);

	CachedDamage        = 0.f;
	CachedDamageGEClass = nullptr;
	SourceASC           = nullptr;
}

// ─────────────────────────────────────────────────────────────────────────────
// 초기화
// ─────────────────────────────────────────────────────────────────────────────

void AEnemyProjectile::InitEnemyProjectile(const FVector& Direction,float Speed,float Lifetime,float Damage,TSubclassOf<UGameplayEffect> InDamageGEClass,UAbilitySystemComponent* InSourceASC)
{
	if (!ensureMsgf(InDamageGEClass, TEXT("AEnemyProjectile::InitEnemyProjectile — DamageGEClass가 nullptr. BP에서 할당 필요.")))
	{
		ReturnToPool();
		return;
	}

	CachedDamage        = Damage;
	CachedDamageGEClass = InDamageGEClass;
	SourceASC           = InSourceASC;

	// 스폰한 에너미(Instigator) 충돌 무시 — 자기 자신 즉시 충돌 방지
	if (AActor* MyInstigator = GetInstigator())
	{
		SphereComp->IgnoreActorWhenMoving(MyInstigator, true);
	}

	// 방향·속도 설정
	ProjectileComp->Velocity = Direction.GetSafeNormal() * Speed;

	// 콜리전 이벤트 등록 (중복 방지)
	SphereComp->OnComponentHit.AddUniqueDynamic(this, &AEnemyProjectile::OnHit);
	SphereComp->OnComponentBeginOverlap.AddUniqueDynamic(this, &AEnemyProjectile::OnBeginOverlap);

	// 수명 타이머
	GetWorldTimerManager().SetTimer(LifetimeTimerHandle,	this, &AEnemyProjectile::OnLifetimeExpired,Lifetime, false);

	KHS_DEBUG(TEXT("%s — 투사체 발사. Speed:%.0f / Lifetime:%.1f / Damage:%.0f"), *GetName(), Speed, Lifetime, Damage);
}

// ─────────────────────────────────────────────────────────────────────────────
// 충돌
// ─────────────────────────────────────────────────────────────────────────────

void AEnemyProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// 벽/지형 Block 충돌 — 데미지 없이 반납
	ReturnToPool();
}

void AEnemyProjectile::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsValid(OtherActor))
	{
		return;
	}

	IAbilitySystemInterface* ASInterface = Cast<IAbilitySystemInterface>(OtherActor);
	if (!ASInterface)
	{
		return;
	}

	UAbilitySystemComponent* TargetASC = ASInterface->GetAbilitySystemComponent();
	if (!TargetASC)
	{
		return;
	}

	if (TargetASC->HasMatchingGameplayTag(RSTags::State_Dead))
	{
		return;
	}

	// 에너미는 통과 — Team_Player에만 데미지
	if (!TargetASC->HasMatchingGameplayTag(RSTags::Team_Player))
	{
		return;
	}

	ApplyDamageToTarget(TargetASC);
	ReturnToPool();
}

// ─────────────────────────────────────────────────────────────────────────────
// 내부 헬퍼
// ─────────────────────────────────────────────────────────────────────────────

void AEnemyProjectile::ApplyDamageToTarget(UAbilitySystemComponent* TargetASC)
{
	if (!CachedDamageGEClass)
	{
		KHS_WARN(TEXT("DamageGEClass가 없음. 데미지 미적용."));
		return;
	}

	UAbilitySystemComponent* EffectSourceASC = SourceASC.Get();
	if (!EffectSourceASC)
	{
		// 발사한 에너미가 이미 사망/풀 반납된 경우 — 타겟 ASC로 대체
		EffectSourceASC = TargetASC;
	}

	FGameplayEffectContextHandle ContextHandle = EffectSourceASC->MakeEffectContext();
	ContextHandle.AddSourceObject(this);

	FGameplayEffectSpecHandle SpecHandle = EffectSourceASC->MakeOutgoingSpec(CachedDamageGEClass, 1.f, ContextHandle);
	if (!SpecHandle.IsValid())
	{
		KHS_WARN(TEXT("GE Spec 생성 실패."));
		return;
	}

	SpecHandle.Data->SetByCallerTagMagnitudes.Add(RSTags::Data_Damage, -CachedDamage);
	TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

	KHS_INFO(TEXT("Enemy 투사체 피격! 데미지: %.0f"), CachedDamage);
}

void AEnemyProjectile::ReturnToPool()
{
	SphereComp->OnComponentHit.RemoveDynamic(this, &AEnemyProjectile::OnHit);
	SphereComp->OnComponentBeginOverlap.RemoveDynamic(this, &AEnemyProjectile::OnBeginOverlap);

	if (UPoolingSubsystem* PoolSys = GetWorld()->GetSubsystem<UPoolingSubsystem>())
	{
		PoolSys->ReturnToPool(this);
	}
}

void AEnemyProjectile::OnLifetimeExpired()
{
	KHS_DEBUG(TEXT("EnemyProjectile 수명 만료 — ReturnToPool."));
	ReturnToPool();
}
