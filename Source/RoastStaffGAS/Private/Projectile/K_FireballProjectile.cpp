// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile/K_FireballProjectile.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystem/GameplayTags/K_GameplayTags.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "System/K_LoggingSystem.h"

// Sets default values
AK_FireballProjectile::AK_FireballProjectile()
	:BaseDamage(10.f), OwnerASC(nullptr), LifeSpan(5.f)
{
	PrimaryActorTick.bCanEverTick = false;

	//SphereComp Setting(물리적 충돌없이 오버랩만 감지 )
	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetSphereRadius(15.f);
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	SphereComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	SetRootComponent(SphereComp);
	
	//ProjectileComp Setting
	ProjectileComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileComp->InitialSpeed = 2000.f;
	ProjectileComp->MaxSpeed = 2000.f;
	ProjectileComp->bRotationFollowsVelocity = true;
	ProjectileComp->ProjectileGravityScale = 0.f; //중력없이 직선이동
	ProjectileComp->bShouldBounce = false;
}

void AK_FireballProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	SphereComp->OnComponentBeginOverlap.AddDynamic(this, &AK_FireballProjectile::OnSphereOverlap);
	
	SetLifeSpan(LifeSpan);
	
	KHS_WARN(TEXT("[Fireball] Spawned - BaseDamage : %.1f"), BaseDamage);
}

void AK_FireballProjectile::SetDamageInfo(float InBaseDamage, UAbilitySystemComponent* InOwnerASC)
{
	BaseDamage = InBaseDamage;
	OwnerASC = InOwnerASC;
}

void AK_FireballProjectile::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!ensureMsgf(OtherActor, TEXT("No Valid Actor")))
	{
		return;
	}
	
	//Ignore Self
	if (OtherActor == this)
	{
		return;
	}
	//Ignore Owner
	if (OtherActor == GetOwner())
	{
		return;
	}
	
	KHS_INFO(TEXT("[Fireball] Overlap with %s"), *OtherActor->GetName());
	
	//Check TargetASC(ASC없는 액터이거나, ASC가 유효하지않은 경우)
	IAbilitySystemInterface* interface = Cast<IAbilitySystemInterface>(OtherActor);
	if (!ensureMsgf(interface, TEXT("[Fireball] Hit non-ASC actor, destroy projectile")))
	{
		Destroy();
		return;
	}
	
	UAbilitySystemComponent* TargetASC = interface->GetAbilitySystemComponent();
	if (!ensureMsgf(TargetASC, TEXT("No Valid ASC")))
	{
		Destroy();
		return;
	}
	
	//데미지/화상 적용
	ApplyDamageAndBurn(OtherActor, TargetASC);
	
	Destroy();
}

void AK_FireballProjectile::ApplyDamageAndBurn(AActor* TargetActor, UAbilitySystemComponent* TargetASC)
{
	if (!TargetASC || !OwnerASC)
	{
		KHS_WARN(TEXT("[Fireball] Invalid ASC of Target & Owner"));
		return;
	}
	
	//Burning 상태 체크
	const bool bIsBurning = TargetASC->HasMatchingGameplayTag(KTags::State_Debuff_Burning);
	
	float FinalDamage = BaseDamage;
	if (bIsBurning)
	{
		FinalDamage = BaseDamage * 2.f;
		KHS_INFO(TEXT("[Fireball] Target is Burning. Damage doubled : %.1f"), FinalDamage);
	}
	else
	{
		KHS_INFO(TEXT("[Fireball] Target is not Burning. Damage : %.1f"), FinalDamage);
	}
	
	//ApplyDamage(ImpactDamage)
	if (ImpactDamageEffect)
	{
		ApplyGameplayEffectToTarget(TargetASC, ImpactDamageEffect, KTags::Data_Damage, FinalDamage);
	}
	else
	{
		KHS_WARN(TEXT("[Fireball] Impact damage effect is disabled"));
	}
	
	//Apply State Burning(GE_Burn)
	if (BurnEffect)
	{
		ApplyGameplayEffectToTarget(TargetASC, BurnEffect);
	}
	else
	{
		KHS_WARN(TEXT("[Fireball] Burn effect is disabled"));
	}
	
	//Apply Burn Damage(Burn Damage)
	if (BurnDamageOverTimeEffect)
	{
		ApplyGameplayEffectToTarget(TargetASC, BurnDamageOverTimeEffect);
	}
	else
	{
		KHS_WARN(TEXT("[Fireball] Burn Damage Over Time is disabled"));
	}
}

bool AK_FireballProjectile::ApplyGameplayEffectToTarget(UAbilitySystemComponent* TargetASC,
	TSubclassOf<UGameplayEffect> EffectClass, FGameplayTag SetByCallerTag, float Magnitude)
{
	if (!TargetASC || !EffectClass || !OwnerASC)
	{
		KHS_WARN(TEXT("[Fireball] Invalid ASC of Target & Owner, EffectClass"));
		return false;
	}
	
	//Create EffectContext
	FGameplayEffectContextHandle contextHandle = OwnerASC->MakeEffectContext();
	contextHandle.AddSourceObject(this); //발사체를 소스오브젝트로 설정
	
	//Set Instigator(데미지를 준 주체)
	AActor* owner = OwnerASC->GetAvatarActor();
	if (owner)
	{
		contextHandle.AddInstigator(owner, this);
	}
	else
	{
		KHS_WARN(TEXT("[Fireball] Invalid AvatarActor for Instigator "));
	}
	
	//Create Effect Spec
	FGameplayEffectSpecHandle specHandle = OwnerASC->MakeOutgoingSpec(EffectClass, 1.f, contextHandle);
	if (!specHandle.IsValid())
	{
		KHS_WARN(TEXT("[Fireball] Failed to make outgoing spec for effect : %s"), *EffectClass->GetName());
		return false;
	}
	
	//SetByCaller 세팅
	if (SetByCallerTag.IsValid())
	{
		specHandle.Data->SetSetByCallerMagnitude(SetByCallerTag, Magnitude);
		KHS_INFO(TEXT("[Fireball] SetByCaller - Tag : %s, Value : %.1f"), *SetByCallerTag.ToString(), Magnitude);
	}
	
	//Apply GE (OwnerASC -> TargetASC)
	FActiveGameplayEffectHandle activeHandle = 
		OwnerASC->ApplyGameplayEffectSpecToTarget(*specHandle.Data.Get(), TargetASC);
	
	const bool bSucess = activeHandle.IsValid();
	
	KHS_INFO(TEXT("[Fireball] Applied %s to %s - %s"), *EffectClass->GetName(), 
		*TargetASC->GetAvatarActor()->GetName(), bSucess ? TEXT("Success") : TEXT("Failure"));
	
	return bSucess;
	
}
