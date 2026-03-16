// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnumTypes.h"
#include "Abilities/GameplayAbility.h"
#include "Engine/DataTable.h" 
#include "BehaviorTree/BehaviorTree.h"
#include "DataTableStructs.generated.h"



// ============================================================================
// DataTable 구조체
// ============================================================================
// ----------------------------------------------------------------------------
// DT_Weapon — 무기 기본 데이터
// ----------------------------------------------------------------------------
USTRUCT(BlueprintType)
struct FWeaponStaticData : public FTableRowBase
{
	GENERATED_BODY()

	/* PK */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	FName WeaponID;
	/** 베이스 타입 (FireBall, IceBall, DarkPierce, Lightning 등) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	FName BaseType;
	/** 무기 강화 레벨 (1~3). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	int32 WeaponLevel;
	/** DT_Skill FK — 이 무기가 사용하는 스킬 ID */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	FName SkillID;
	/** 슬롯 타입 (Main / Sub1 / Sub2) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	EWeaponSlotType SlotType;
	/** 강화 시 다음 단계 무기 ID. 최대 레벨이면 None */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	FName NextLevelWeaponID;
	/** DT_String FK — 무기 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	FName WeaponName;
	/** DT_String FK — 무기 설명 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	FName Description;
};


// ----------------------------------------------------------------------------
// DT_Skill — 스킬 기본 데이터 (GA/GE 클래스 매핑)
// ----------------------------------------------------------------------------
class ABaseProjectile;

USTRUCT(BlueprintType)
struct FSkillStaticData : public FTableRowBase
{
	GENERATED_BODY()

	/** PK */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	FName SkillID;
	/** DT_SkillEffect FK */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	FName SkillEffectID;
	/** DT_SkillResource FK (FX용) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	FName ResourceID;
	/** 스킬 아이콘 텍스처 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	TSoftObjectPtr<UTexture2D> SkillIcon;
	/** GA 클래스 에셋 경로 (데이터 드리븐 로딩용) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	TSoftClassPtr<UGameplayAbility> GAClass;
	/** 투사체 액터 클래스 경로. 소환형은 nullptr */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	TSoftClassPtr<ABaseProjectile> ProjectileClass;
	/** 데미지 GE 클래스 경로 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	TSoftClassPtr<UGameplayEffect> DamageGEClass;
	/** 상태이상 GE 클래스 경로. 없으면 nullptr */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	TSoftClassPtr<UGameplayEffect> StatusGEClass;
	/** 쿨타임 GE 에셋 경로 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Future")
	TSoftClassPtr<UGameplayEffect> CooldownGE;
	/** 코스트 GE 에셋 경로 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Future")
	TSoftClassPtr<UGameplayEffect> CostGE;
	/** 스킬 태그 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Future")
	FGameplayTagContainer Tags;
};


// ----------------------------------------------------------------------------
// DT_SkillEffect — 스킬 효과 공통 파라미터
// ----------------------------------------------------------------------------
USTRUCT(BlueprintType)
struct FSkillEffectData : public FTableRowBase
{
	GENERATED_BODY()

	/** PK */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SkillEffect")
	FName SkillEffectID;
	/** DT_StatusEffect FK. 없으면 None */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SkillEffect")
	FName StatusEffectID;
	/** 스킬 타입 (Projectile / Summon) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SkillEffect")
	ESkillType SkillType;
	/** 투사체 비행 방식. 소환형이면 None */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SkillEffect")
	EFlightType FlightType;
	/** 타격 방식 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SkillEffect")
	EHitType HitType;
	/** 기본 데미지 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SkillEffect")
	float Damage = 0.f;
	/** 투사체 이동 속도. 소환형은 0 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SkillEffect")
	float Speed = 0.f;
	/** 투사체 최대 생존 시간 (초). 소환형은 0 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SkillEffect")
	float Lifetime = 0.f;
	/** 투사체 소멸 조건 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SkillEffect")
	EExpireCondition ExpireCondition;
	/** 스킬 쿨타임 (초). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SkillEffect")
	float Cooldown = 1.f;
};


// ----------------------------------------------------------------------------
// DT_Spawn — 투사체 스폰 파라미터
// ----------------------------------------------------------------------------
USTRUCT(BlueprintType)
struct FSkillSpawnData : public FTableRowBase
{
	GENERATED_BODY()

	/** PK  */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	FName SkillEffectID;
	/** 발사당 생성 투사체 수 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	int32 ProjectileCount = 1;
	/** 생성 소켓 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	FName SocketName;
	/** 스폰 패턴 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	ESpawnPattern SpawnPattern;
	/** Spread 전용 — 부채꼴 전체 각도 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	float SpreadAngle = 0.f;
};


// ----------------------------------------------------------------------------
// DT_Flight_Arc — 곡사형 전용 파라미터
// ----------------------------------------------------------------------------
USTRUCT(BlueprintType)
struct FFlightArcData : public FTableRowBase
{
	GENERATED_BODY()

	/** PK  */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flight|Arc")
	FName SkillEffectID;
	/** 초기 발사 각도 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flight|Arc")
	float LaunchAngle;
	/** 중력 배율 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flight|Arc")
	float GravityScale;
	/** 타격 판정 반경 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flight|Arc")
	float HitRadius;
	/** 최대 타격 대상 수. 0이면 범위 내 전체 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flight|Arc")
	int32 MaxTargetCount;
};


// ----------------------------------------------------------------------------
// DT_Flight_Pierce — 관통형 전용 파라미터
// ----------------------------------------------------------------------------
USTRUCT(BlueprintType)
struct FFlightPierceData : public FTableRowBase
{
	GENERATED_BODY()

	/** PK */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flight|Pierce")
	FName SkillEffectID;
	/** 최대 관통 횟수 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flight|Pierce")
	int32 PierceCount;
	/** 관통마다 적용되는 데미지 감쇠율 (0.0~1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flight|Pierce")
	float DamageDecay;
};


// ----------------------------------------------------------------------------
// DT_Flight_Explode — 폭발형 전용 파라미터
// ----------------------------------------------------------------------------
USTRUCT(BlueprintType)
struct FFlightExplodeData : public FTableRowBase
{
	GENERATED_BODY()

	/** PK  */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flight|Explode")
	FName SkillEffectID;
	/** 폭발 피해 반경 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flight|Explode")
	float ExplosionRadius;
	/** 거리 기반 데미지 감쇠 배율 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flight|Explode")
	float ExplosionDamageMultiplier;
	/** 타격 판정 반경 (직접 충돌용) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flight|Explode")
	float HitRadius;
	/** 최대 타격 대상 수. 0이면 범위 내 전체 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flight|Explode")
	int32 MaxTargetCount;
};



// ----------------------------------------------------------------------------
// DT_Enemy — 에너미 기본 데이터
// ----------------------------------------------------------------------------
USTRUCT(BlueprintType)
struct FEnemyStaticData : public FTableRowBase
{
	GENERATED_BODY()

	/** PK */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	FName EnemyID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	float MaxHP;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	float MoveSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	float AttackDamage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	float AttackCooldown;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	float AttackRange;
	
	/** AI 행동 패턴 타입 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	EAIType AIType;

	/** BehaviorTree 에셋 경로 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	TSoftObjectPtr<UBehaviorTree> BehaviorTree;

	/** 웨이브 스폰 가중치 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	float SpawnWeight;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	int32 DropEXP;
};



// ----------------------------------------------------------------------------
// DT_Stage — 스테이지 기본 데이터
// ----------------------------------------------------------------------------
USTRUCT(BlueprintType)
struct FStageStaticData : public FTableRowBase
{
	GENERATED_BODY()

	/** PK */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stage")
	FName StageID;
	/** 스테이지 제한 시간 (초). 클리어 조건 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stage")
	float TimeLimit;
	/** 이 스테이지에서 스폰 가능한 에너미 ID 목록 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stage")
	TArray<FName> SpawnEnemyIDs;
};



// ----------------------------------------------------------------------------
// DT_WaveData — 웨이브 구간 데이터
// StageID FK로 스테이지와 연결
// ----------------------------------------------------------------------------
USTRUCT(BlueprintType)
struct FWaveStaticData : public FTableRowBase
{
	GENERATED_BODY()

	/** 소속 스테이지 (FK) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave")
	FName StageID;
	/** 웨이브 구간 번호 (0부터) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave")
	int32 WaveIndex;
	/** 웨이브 구간 시작 경과 시간 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave")
	float StartTime;
	/** 적 스폰 간격 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave")
	float SpawnInterval;
	/** 최대 동시 생존 적 수 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave")
	int32 MaxAliveCount;
	/** 스폰 가능한 적 ID 목록 (DT_Enemy FK) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave")
	TArray<FName> SpawnEnemyIDs;
	/** SpawnEnemyIDs별 스폰 가중치  */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave")
	TArray<float> SpawnWeights;
};