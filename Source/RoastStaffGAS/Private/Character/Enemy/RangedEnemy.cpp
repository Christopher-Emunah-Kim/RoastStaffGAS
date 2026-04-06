// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Enemy/RangedEnemy.h"
#include "Objects/Projectile/EnemyProjectile.h"
#include "Subsystems/PoolingSubsystem.h"
#include "RoastStaffGAS.h"

ARangedEnemy::ARangedEnemy()
{
}

// ─────────────────────────────────────────────────────────────────────────────
// 초기화
// ─────────────────────────────────────────────────────────────────────────────

void ARangedEnemy::InitializeRangedParams(float InAttackDamage, const FEnemyExtData& ExtData)
{
	// DT_Enemy(StaticData) → AttackDamage / DT_EnemyExtData → 원거리 파라미터
	AttackDamage      = InAttackDamage;
	PreferredRange    = ExtData.PreferredRange;
	MaxAttackRange    = ExtData.MaxAttackRange;
	ProjectileSpeed   = ExtData.ProjectileSpeed;
	ProjectileLifetime = ExtData.ProjectileLifetime;

	KHS_INFO(TEXT("%s — RangedParams 초기화. ATK: %.0f / PreferredRange: %.0f / MaxRange: %.0f / Speed: %.0f"),
		*GetName(), AttackDamage, PreferredRange, MaxAttackRange, ProjectileSpeed);
}

// ─────────────────────────────────────────────────────────────────────────────
// 투사체 발사
// ─────────────────────────────────────────────────────────────────────────────

void ARangedEnemy::FireProjectile()
{
	if (!AttackGEClass)
	{
		KHS_WARN(TEXT("%s — AttackGEClass 미할당. BP_RangedEnemy에서 설정 필요."), *GetName());
		return;
	}

	if (!ProjectileClass)
	{
		KHS_WARN(TEXT("%s — ProjectileClass 미할당. BP_RangedEnemy에서 설정 필요."), *GetName());
		return;
	}

	// 플레이어 방향 계산
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC || !PC->GetPawn())
	{
		KHS_WARN(TEXT("%s — 플레이어 폰 없음. 투사체 발사 취소."), *GetName());
		return;
	}

	const FVector TargetLocation = PC->GetPawn()->GetActorLocation();
	const FVector Direction = (TargetLocation - GetActorLocation()).GetSafeNormal();

	LaunchProjectile(Direction);
}

void ARangedEnemy::LaunchProjectile(const FVector& Direction)
{
	UPoolingSubsystem* PoolSys = GetWorld()->GetSubsystem<UPoolingSubsystem>();
	if (!PoolSys)
	{
		KHS_WARN(TEXT("%s — PoolingSubsystem 없음."), *GetName());
		return;
	}

	const FTransform SpawnTransform(FRotator::ZeroRotator, GetActorLocation());
	AEnemyProjectile* Projectile = PoolSys->SpawnPooledActor<AEnemyProjectile>(ProjectileClass, SpawnTransform);
	if (!Projectile)
	{
		KHS_WARN(TEXT("%s — EnemyProjectile 풀 고갈. 발사 스킵."), *GetName());
		return;
	}

	Projectile->InitEnemyProjectile(Direction,	ProjectileSpeed, ProjectileLifetime,
		AttackDamage, AttackGEClass,	GetAbilitySystemComponent());

	KHS_DEBUG(TEXT("%s — 투사체 발사. 방향: %s"), *GetName(), *Direction.ToString());
}
