// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

// ============================================================================
// ENUM 정의
// ============================================================================

/** 무기 슬롯 타입 */
UENUM(BlueprintType)
enum class EWeaponSlotType : uint8
{
	MAIN		UMETA(DisplayName = "Main"),
	SUB01		UMETA(DisplayName = "Sub01"),
	SUB02		UMETA(DisplayName = "Sub02")
};

/** 스킬 타입 (투사체 / 소환) */
UENUM(BlueprintType)
enum class ESkillType : uint8
{
	PROJECTILE	UMETA(DisplayName = "Projectile"),
	SUMMON		UMETA(DisplayName = "Summon")
};

/** 투사체 비행 방식 */
UENUM(BlueprintType)
enum class EFlightType : uint8
{
	NONE		UMETA(DisplayName = "None"),
	LINEAR		UMETA(DisplayName = "Linear"),
	HOMING		UMETA(DisplayName = "Homing"),
	ARC			UMETA(DisplayName = "Arc"),
	PIERCE		UMETA(DisplayName = "Pierce"),
	EXPLODE		UMETA(DisplayName = "Explode")
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
	OnExplosion		UMETA(DisplayName = "OnExplosion"),
	OnPierceCount	UMETA(DisplayName = "OnPierceCount"),
	OnLifetime		UMETA(DisplayName = "OnLifetime")
};

/** 투사체 스폰 패턴 */
UENUM(BlueprintType)
enum class ESpawnPattern : uint8
{
	SINGLE		UMETA(DisplayName = "Single"),
	SPREAD		UMETA(DisplayName = "FanSpread"),
	CIRCLE		UMETA(DisplayName = "Circle")
};

/** 에너미 AI 행동 패턴 타입 */
UENUM(BlueprintType)
enum class EAIType : uint8
{
	CHASE	UMETA(DisplayName = "Chase"),
	RANGED	UMETA(DisplayName = "Ranged"),
	ELITE	UMETA(DisplayName = "Elite")
};