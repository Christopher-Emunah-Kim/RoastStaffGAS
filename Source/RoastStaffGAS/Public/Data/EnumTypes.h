// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnumTypes.generated.h"

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
	MELEE			UMETA(DisplayName = "Melee"),
	LINEAR			UMETA(DisplayName = "Linear"),
	HOMING			UMETA(DisplayName = "Homing"),
	HOMING_BOUNCE	UMETA(DisplayName = "HomingBounce"),  // 유도 + 적 충돌 시 다음 적으로 바운스
	ARC				UMETA(DisplayName = "Arc"),
	SUMMON			UMETA(DisplayName = "Summon"),
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
	ELITE	UMETA(DisplayName = "Elite"),
	BOSS	UMETA(DisplayName = "Boss")
};

/** 캐릭터 등급 — 그리드 정렬 2순위 기준 (SSR > SR > R > N) */
UENUM(BlueprintType)
enum class ECharacterGrade : uint8
{
	SSR  UMETA(DisplayName = "SSR"),
	SR   UMETA(DisplayName = "SR"),
	R    UMETA(DisplayName = "R"),
	N    UMETA(DisplayName = "N"),
};

/** 캐릭터 해금 조건 타입 */
UENUM(BlueprintType)
enum class ECharacterUnlockType : uint8
{
	DEFAULT      UMETA(DisplayName = "Default"),      // 기본 해금 (조건 없음)
	STAGE_CLEAR  UMETA(DisplayName = "StageClear"),   // 특정 스테이지 클리어 시 해금
	CURRENCY     UMETA(DisplayName = "Currency"),     // 재화 소모 해금 (stub — 재화 시스템 미구현)
};

/** 스테이지 노드 진입 가능 상태 */
UENUM(BlueprintType)
enum class EStageNodeState : uint8
{
	AVAILABLE  UMETA(DisplayName = "Available"),   // 진입 가능 (미클리어)
	CLEARED    UMETA(DisplayName = "Cleared"),     // 클리어 완료
	LOCKED     UMETA(DisplayName = "Locked"),      // 해금 조건 미충족
};

/**
 * 캐릭터 고유 스킬 발동 방식
 * @deprecated PLAN_SkillActivationRefactor — ESkillTargetingType + ESkillEffectType으로 대체.
 *             무기 스킬 격리 완료 후 삭제 예정.
 */
UENUM(BlueprintType)
enum class ESkillActivationType : uint8
{
	InstantAoE      UMETA(DisplayName = "InstantAoE"),      // 즉발 범위기 — 캐릭터 주변 즉시 발동
	SpawnPreview    UMETA(DisplayName = "SpawnPreview"),    // 위치 지정형 — 프리뷰 FX → 좌클릭 확정
	SelfBuff        UMETA(DisplayName = "SelfBuff"),        // 자가 버프 — 자신에게 즉시 GE 적용
	ProjectileSpawn UMETA(DisplayName = "ProjectileSpawn"), // 투사체 직접 발사 — ProjectileCount × FireInterval
	GroundEffect    UMETA(DisplayName = "GroundEffect"),    // 장판 배치 — ARS_GroundEffectActor 스폰
};

/** 조준방식 — 스킬 확정까지의 입력 흐름 */
UENUM(BlueprintType)
enum class ESkillTargetingType : uint8
{
	Instant           UMETA(DisplayName = "Instant"),           // 즉시 발동 (위치 지정 없음)
	AimPreview        UMETA(DisplayName = "AimPreview"),        // 프리뷰 위치 지정 후 확정
	LaunchProjectile  UMETA(DisplayName = "LaunchProjectile"),  // 투사체 발사
	ChargeAndRelease  UMETA(DisplayName = "ChargeAndRelease"),  // 차징 후 발사 [Stub — DEFERRED]
};

/** 효과 타입 — 스킬 확정 후 실제 결과 */
UENUM(BlueprintType)
enum class ESkillEffectType : uint8
{
	RadialAoE   UMETA(DisplayName = "RadialAoE"),   // 범위 GE
	SelfBuff    UMETA(DisplayName = "SelfBuff"),    // 자신 GE
	Teleport    UMETA(DisplayName = "Teleport"),    // 확정 위치로 이동
	SpawnActor  UMETA(DisplayName = "SpawnActor"),  // EffectActorClass 스폰 + InitEffect
	Projectile  UMETA(DisplayName = "Projectile"),  // 투사체 (ProjectileMoveType으로 세분화)
};

/** 투사체 이동+착탄 방식 — EffectType == Projectile 시만 유효 */
UENUM(BlueprintType)
enum class EProjectileMoveType : uint8
{
	Linear       UMETA(DisplayName = "Linear"),       // 직선
	Pierce       UMETA(DisplayName = "Pierce"),       // 직선 관통
	Homing       UMETA(DisplayName = "Homing"),       // 유도
	HomingBounce UMETA(DisplayName = "HomingBounce"), // 유도 + 바운스
	Explode      UMETA(DisplayName = "Explode"),      // 착탄 폭발 AoE
};

/** 투사체 발사 패턴 */
UENUM(BlueprintType)
enum class ESkillSpawnPattern : uint8
{
	Single  UMETA(DisplayName = "Single"),  // 단발
	Burst   UMETA(DisplayName = "Burst"),   // 순차 연속 (FireInterval 간격)
	Spread  UMETA(DisplayName = "Spread"),  // 동시 부채꼴
	Circle  UMETA(DisplayName = "Circle"),  // 동시 원형
};

/** 레벨업 카드 타입 */
UENUM(BlueprintType)
enum class ELevelUpCardType : uint8
{
	StatUpgrade   UMETA(DisplayName = "StatUpgrade"),   // 스탯 업그레이드 (정적 카드)
	PassiveAdd    UMETA(DisplayName = "PassiveAdd"),    // 패시브 스킬 추가 (정적 카드, 슬롯 < 4 조건)
	WeaponUpgrade UMETA(DisplayName = "WeaponUpgrade"), // 장착 무기 강화 (동적 카드)
	WeaponNew     UMETA(DisplayName = "WeaponNew"),     // 새 무기 획득 (동적 카드)
};
