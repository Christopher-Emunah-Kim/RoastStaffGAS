// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Enemy/RangedEnemy.h"
#include "RoastStaffGAS.h"
#include "Objects/Projectile/EnemyProjectile.h"

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
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC || !PC->GetPawn())
	{
		KHS_WARN(TEXT("%s — 플레이어 폰 없음. 투사체 발사 취소."), *GetName());
		return;
	}

	const FVector Direction = (PC->GetPawn()->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	LaunchEnemyProjectile(Direction, AttackDamage);
}
