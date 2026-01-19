// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile/K_BasicShootProjectile.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffectTypes.h"
#include "NavigationSystemTypes.h"
#include "AbilitySystem/GameplayTags/K_GameplayTags.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "System/K_LoggingSystem.h"

// Sets default values
AK_BasicShootProjectile::AK_BasicShootProjectile()
	: OwnerASC(nullptr), BaseDamage(5.f), LifeSpan(3.f)
{
	//tick 비활성
	PrimaryActorTick.bCanEverTick = false;

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SetRootComponent(SphereComp);
	SphereComp->SetSphereRadius(15.f);
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComp->SetCollisionResponseToAllChannels(ECR_Ignore); 
	SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap); //Pawn에만 overlap
	SphereComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	SphereComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	
	ProjectileComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileComp->UpdatedComponent = SphereComp;
	ProjectileComp->InitialSpeed = 3000.f;
	ProjectileComp->MaxSpeed = 3000.f;
	ProjectileComp->bRotationFollowsVelocity = true;
	ProjectileComp->ProjectileGravityScale = 0.f;
	ProjectileComp->bShouldBounce = false;
}

// Called when the game starts or when spawned
void AK_BasicShootProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	SphereComp->OnComponentBeginOverlap.AddDynamic(this, &AK_BasicShootProjectile::OnSphereOverlap);
	SetLifeSpan(LifeSpan);
	
	KHS_INFO(TEXT("[BasicProjectile] Spawned - BaseDamage %1.f, LifeSpan : %1.f"), BaseDamage, LifeSpan);
}

void AK_BasicShootProjectile::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//유효성 확인
	if (!ensureMsgf(OtherActor, TEXT("[BasicProjectile] Overlap with null Actor")))
	{
		return;
	}
	
	if (OtherActor == this || OtherActor == GetOwner())
	{
		return;
	}
	
	//GAS 인터페이스 보유여부 확인
	IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(OtherActor);
	if (!ASCInterface)
	{
		KHS_INFO(TEXT("[BasicProjectile] Non-ASC Actor, Destroying.."));
		Destroy();
		return;
	}
	
	UAbilitySystemComponent* TargetASC = ASCInterface->GetAbilitySystemComponent();
	if (!ensureMsgf(TargetASC, TEXT("[BasicProjectile] Target has Interface but no ASC")))
	{
		Destroy();
		return;
	}
	
	//데미지 적용
	const bool bSuccess = ApplyDamageToTarget(TargetASC);
	
	if (bSuccess)
	{
		KHS_INFO(TEXT("[BasicProjectile] Damage Applied to %s"), *OtherActor->GetName());
	}
	else
	{
		KHS_WARN(TEXT("[BasicProjectile] Failed to apply damage to %s"), *OtherActor->GetName());
	}
	
	//발사체 파괴
	Destroy();
}

bool AK_BasicShootProjectile::ApplyDamageToTarget(UAbilitySystemComponent* TargetASC)
{
	if (!ensureMsgf(TargetASC, TEXT("[BasicProjectile] TargetASC is NULL")))
	{
		return false;
	}
	
	if (!ensureMsgf(OwnerASC, TEXT("[BasicProjectile] OwnerASC is NULL")))
	{
		return false;
	}
	
	if (!ensureMsgf(DamageEffect, TEXT("[BasicProjectile] DamageEffect is not set in BP")))
	{
		return false;
	}
	
	//EffectContext 생성(GE 사용처 설정)
	FGameplayEffectContextHandle contextHandle = OwnerASC->MakeEffectContext();
	contextHandle.AddSourceObject(this);
	
	AActor* instigator = OwnerASC->GetAvatarActor();
	if (!ensureMsgf(instigator, TEXT("[BasicProjectile] GetAvatarActor is failed")))
	{
		return false;
	}
	contextHandle.AddInstigator(instigator, this); //가해자 : 오너액터, 원인 : 발사체
	
	//EffectSpec 생성(GE context handle 기반으로 인스턴스 정보 생성)
	FGameplayEffectSpecHandle specHandle = OwnerASC->MakeOutgoingSpec(DamageEffect, 1.f, contextHandle);
	
	if (!specHandle.IsValid())
	{
		KHS_WARN(TEXT("[BaiscProjectile] Failed to Create EffectSpec"));
		return false;
	}
	
	//SetByCaller로 값 설정
	specHandle.Data->SetSetByCallerMagnitude(KTags::Data_Damage, BaseDamage); 
	
	//GE 적용 (성공하면 activeHandle생성)
	FActiveGameplayEffectHandle activeHandle = OwnerASC->ApplyGameplayEffectSpecToTarget(
		*specHandle.Data.Get(), TargetASC);
	
	//activeHandle 유효성 확인해서 GE 적용성공여부 확인
	const bool bSuccess = activeHandle.IsValid();
	return bSuccess;
	
	KHS_INFO(TEXT("[BasicProjectile] ApplyGE Result : %s"), bSuccess? TEXT("Success") : TEXT("Failed"));
}

void AK_BasicShootProjectile::SetDamageInfo(float InBaseDamage, UAbilitySystemComponent* InOwnerASC)
{
	BaseDamage = InBaseDamage;
	OwnerASC = InOwnerASC;
	
	KHS_INFO(TEXT("[BasicProjectile] Damage Info is set - Damage - %.1f, OwnerASC - %s"), BaseDamage, OwnerASC? TEXT("Valid") : TEXT("Invalid"));
}

