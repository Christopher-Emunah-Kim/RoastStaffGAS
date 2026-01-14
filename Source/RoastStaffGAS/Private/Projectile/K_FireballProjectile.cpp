// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile/K_FireballProjectile.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

// Sets default values
AK_FireballProjectile::AK_FireballProjectile()
	:BaseDamage(10.f), OwnerASC(nullptr)
{
	PrimaryActorTick.bCanEverTick = false;

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetSphereRadius(15.f);
	SphereComp->SetCollisionProfileName(TEXT("Projectile"));
	SetRootComponent(SphereComp);
	
	ProjectileComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileComp->InitialSpeed = 2000.f;
	ProjectileComp->MaxSpeed = 2000.f;
	ProjectileComp->bRotationFollowsVelocity = true;
	ProjectileComp->ProjectileGravityScale = 0.f;
}

void AK_FireballProjectile::BeginPlay()
{
	Super::BeginPlay();
	
}

void AK_FireballProjectile::SetDamageInfo(float InBaseDamage, UAbilitySystemComponent* InOwnerASC)
{
	BaseDamage = InBaseDamage;
	OwnerASC = InOwnerASC;
}
