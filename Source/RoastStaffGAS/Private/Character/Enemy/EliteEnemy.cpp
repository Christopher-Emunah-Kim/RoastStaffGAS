// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Enemy/EliteEnemy.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"
#include "GAS/Tags/RSGameplayTags.h"
#include "RoastStaffGAS.h"
#include "Objects/Projectile/EnemyProjectile.h"

AEliteEnemy::AEliteEnemy()
{
	// 돌진 피해 감지 구체 — 기본 비활성(돌진 시만 활성)
	ChargeHitSphere = CreateDefaultSubobject<USphereComponent>(TEXT("ChargeHitSphere"));
	ChargeHitSphere->SetupAttachment(RootComponent);
	ChargeHitSphere->SetSphereRadius(80.f);
	ChargeHitSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

// ─────────────────────────────────────────────────────────────────────────────
// 초기화
// ─────────────────────────────────────────────────────────────────────────────

void AEliteEnemy::BeginPlay()
{
	Super::BeginPlay();
	ChargeHitSphere->OnComponentBeginOverlap.AddUniqueDynamic(this, &AEliteEnemy::OnChargeHitBeginOverlap);
}

void AEliteEnemy::InitializeEliteParams(float InAttackDamage, const FEnemyExtData& ExtData)
{
	AttackDamage      = InAttackDamage;
	PreferredRange    = ExtData.PreferredRange;
	MaxAttackRange    = ExtData.MaxAttackRange;
	ProjectileSpeed   = ExtData.ProjectileSpeed;
	ProjectileLifetime = ExtData.ProjectileLifetime;

	KHS_DEBUG(TEXT("%s — EliteParams 초기화. ATK: %.0f / PreferredRange: %.0f / MaxRange: %.0f"),
		*GetName(), AttackDamage, PreferredRange, MaxAttackRange);
}

// ─────────────────────────────────────────────────────────────────────────────
// 투사체 발사 (원거리 기본 루프)
// ─────────────────────────────────────────────────────────────────────────────

void AEliteEnemy::FireProjectile()
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC || !PC->GetPawn())
	{
		KHS_WARN(TEXT("%s — 플레이어 폰 없음. 투사체 발사 취소."), *GetName());
		return;
	}

	const FVector Direction = (PC->GetPawn()->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	LaunchEnemyProjectile(Direction, AttackDamage);
}

// ─────────────────────────────────────────────────────────────────────────────
// 근접 돌진
// ─────────────────────────────────────────────────────────────────────────────

void AEliteEnemy::MeleeCharge()
{
	if (bIsCharging)
	{
		return;
	}

	bIsCharging = true;

	// 이동 속도 부스트
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		BaseWalkSpeed = MoveComp->MaxWalkSpeed;
		MoveComp->MaxWalkSpeed = BaseWalkSpeed * ChargeSpeedMult;
	}

	// 돌진 피해 감지 활성
	ChargeHitSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	KHS_DEBUG(TEXT("%s — 근접 돌진 시작."), *GetName());
}

void AEliteEnemy::EndCharge()
{
	if (!bIsCharging)
	{
		return;
	}

	bIsCharging = false;

	// 이동 속도 복원
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = BaseWalkSpeed;
	}

	// 돌진 피해 감지 비활성
	ChargeHitSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	KHS_DEBUG(TEXT("%s — 근접 돌진 종료."), *GetName());
}


void AEliteEnemy::OnChargeHitBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	
	if (!OtherActor || !bIsCharging)
	{
		return;
	}

	IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(OtherActor);
	if (!ASCInterface)
	{
		return;
	}

	UAbilitySystemComponent* TargetASC = ASCInterface->GetAbilitySystemComponent();
	if (!TargetASC || !TargetASC->HasMatchingGameplayTag(RSTags::Team_Player))
	{
		return;
	}

	ApplyChargeDamage(OtherActor);
	EndCharge();
}


void AEliteEnemy::ApplyChargeDamage(AActor* Target)
{
	if (!ChargeGEClass)
	{
		KHS_WARN(TEXT("%s — ChargeGEClass 미할당."), *GetName());
		return;
	}

	IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(Target);
	if (!ASCInterface)
	{
		return;
	}

	UAbilitySystemComponent* TargetASC = ASCInterface->GetAbilitySystemComponent();
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponent();
	if (!TargetASC)
	{
		return;
	}

	UAbilitySystemComponent* EffectSource = SourceASC ? SourceASC : TargetASC;
	FGameplayEffectContextHandle Context  = EffectSource->MakeEffectContext();
	FGameplayEffectSpecHandle    Spec     = EffectSource->MakeOutgoingSpec(	ChargeGEClass, 1.f, Context);

	if (!Spec.IsValid())
	{
		return;
	}

	Spec.Data->SetByCallerTagMagnitudes.Add(RSTags::Data_EnemyAttackDamage, AttackDamage * ChargeDamageMult);
	TargetASC->ApplyGameplayEffectSpecToSelf(*Spec.Data);

	KHS_DEBUG(TEXT("%s — 돌진 데미지 적용. 대상: %s / 데미지: %.0f"), *GetName(), *Target->GetName(), AttackDamage * ChargeDamageMult);
}

// ─────────────────────────────────────────────────────────────────────────────
// 사망
// ─────────────────────────────────────────────────────────────────────────────

void AEliteEnemy::HandleDeath()
{
	// 돌진 중 사망 시 상태 즉시 초기화
	EndCharge();
	Super::HandleDeath();
}
