// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

// ============================================================================
// ENUM 정의
// ============================================================================

/** 레벨업 UI 무기 카드 표시 상태 */
UENUM(BlueprintType)
enum class EWeaponCardState : uint8
{
	New      UMETA(DisplayName = "NEW"),      // 미보유 신규 무기
	Lv1ToLv2 UMETA(DisplayName = "Lv1->Lv2"), // 동일 BaseType Lv1 보유 중 — 강화 가능
	Lv2ToLv3 UMETA(DisplayName = "Lv2->Lv3"), // 동일 BaseType Lv2 보유 중 — 강화 가능
	Lv3Max   UMETA(DisplayName = "Lv3(MAX)"), // 동일 BaseType Lv3 보유 — 최대 레벨
};

/** 무기 베이스 타입 */
UENUM(BlueprintType)
enum class EWeaponBaseType : uint8
{
	Fireball		UMETA(DisplayName = "Fireball"),
	Iceball			UMETA(DisplayName = "Iceball"),
	Thunder			UMETA(DisplayName = "Thunder"),
	Poison			UMETA(DisplayName = "Poison"),
	Shadow			UMETA(DisplayName = "Shadow"),
	SummonSword		UMETA(DisplayName = "SummonSword"),
	SummonLightning UMETA(DisplayName = "SummonLightning"),
};

/** 스킬 타입 (투사체 / 소환) */
UENUM(BlueprintType)
enum class ESkillType : uint8
{
	ATTACK		UMETA(DisplayName = "Attack"),
	DEFENSE		UMETA(DisplayName = "Defense"),
	UTILITY		UMETA(DisplayName = "Utility"),
};

/** 투사체 비행 방식 */
UENUM(BlueprintType)
enum class EMoveType : uint8
{
	MELEE		UMETA(DisplayName = "Melee"),
	LINEAR		UMETA(DisplayName = "Linear"),
	HOMING		UMETA(DisplayName = "Homing"),
	ARC			UMETA(DisplayName = "Arc"),
	SUMMON		UMETA(DisplayName = "Summon"),
};

/** 타격 방식 */
UENUM(BlueprintType)
enum class EHitType : uint8
{
	SINGLE		UMETA(DisplayName = "Single"),
	PIERCE		UMETA(DisplayName = "Pierce"),
	AREA		UMETA(DisplayName = "Area")
};

/** 투사체 소멸 조건 */
UENUM(BlueprintType)
enum class EExpireCondition : uint8
{
	OnHit			UMETA(DisplayName = "OnHit"),
	OnLifetime		UMETA(DisplayName = "OnLifetime")
};

/** 투사체 스폰 패턴 */
UENUM(BlueprintType)
enum class ESpawnPattern : uint8
{
	SINGLE		UMETA(DisplayName = "Single"),
	SPREAD		UMETA(DisplayName = "Spread"),
	CIRCLE		UMETA(DisplayName = "Circle")
};

/** 방어 스킬 타입 */
UENUM(BlueprintType)
enum class EDefenseType : uint8
{
	INVINCIBLE	UMETA(DisplayName = "Invincible"),
	ARMOR		UMETA(DisplayName = "Armor"),
};

/** 에너미 AI 행동 패턴 타입 */
UENUM(BlueprintType)
enum class EAIType : uint8
{
	CHASE	UMETA(DisplayName = "Chase"),
	RANGED	UMETA(DisplayName = "Ranged"),
	ELITE	UMETA(DisplayName = "Elite")
};
