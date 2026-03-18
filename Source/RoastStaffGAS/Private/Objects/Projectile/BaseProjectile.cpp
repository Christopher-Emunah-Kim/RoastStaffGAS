// Fill out your copyright notice in the Description page of Project Settings.


#include "Objects/Projectile/BaseProjectile.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameplayEffect.h"
#include "GAS/Tags/RSGameplayTags.h"
#include "System/LoggingSystem.h"

ABaseProjectile::ABaseProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetSphereRadius(50.f);
	SphereComp->SetCollisionProfileName(TEXT("Projectile"));
	RootComponent = SphereComp;

	ProjectileComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileComp->InitialSpeed  = 1000.f;
	ProjectileComp->MaxSpeed      = 3000.f;
	ProjectileComp->bRotationFollowsVelocity = true;
	ProjectileComp->ProjectileGravityScale   = 0.f;
}

void ABaseProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	SphereComp->OnComponentHit.AddDynamic(this, &ABaseProjectile::OnHit);
}

void ABaseProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	if (!OtherActor || OtherActor == GetOwner())
	{
		return;
	}
	
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
	
	if (!TargetASC) //GAS오브젝트가 아님.
	{
		return;
	}
	
	const bool bHasEnemyTag = TargetASC->HasMatchingGameplayTag(RSTags::Team_Enemy);
	// 자식 충돌 처리 — true 반환 시 베이스가 GE처리하고 Destroy
	const bool bShouldDestroy = OnProjectileHit(OtherActor, Hit);

	if (bHasEnemyTag && bShouldDestroy)
	{
		
		// 기본 단일 타격 처리
		if (InitData.DamageGEClass)
		{
			ApplyEffectToTarget(TargetASC, InitData.DamageGEClass, InitData.Amount);
		}

		// 상태이상 처리
		if (InitData.StatusGEClass)
		{
			ApplyEffectToTarget(TargetASC, InitData.StatusGEClass, 0.f);
		}

		GetWorldTimerManager().ClearTimer(LifetimeTimerHandle);
		Destroy();
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

void ABaseProjectile::OnLifetimeExpired()
{
	OnProjectileExpired();
	Destroy();
}

void ABaseProjectile::InitProjectile(const FProjectileInitData& InInitData)
{
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

