// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnumTypes.h"
#include "EnumUITypes.h"
#include "Abilities/GameplayAbility.h"
#include "Engine/DataTable.h" 
#include "BehaviorTree/BehaviorTree.h"
#include "DataTableStructs.generated.h"



// ============================================================================
// DataTable 구조체
// ============================================================================


// ----------------------------------------------------------------------------
// DT_CharacterStatic — 캐릭터 정적 데이터
// 에셋 경로: Content/Data/Character/DT_CharacterStatic
// ----------------------------------------------------------------------------
class USkeletalMesh;
class UAnimInstance;

USTRUCT(BlueprintType)
struct FCharacterStaticData : public FTableRowBase
{
	GENERATED_BODY()

	/** PK */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	FName CharacterID = NAME_None;
	/** 캐릭터 표시 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	FText DisplayName = FText::GetEmpty();

	/** 스켈레탈 메시 에셋 경로 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	TSoftObjectPtr<USkeletalMesh> Mesh = nullptr;
	/** 애님 블루프린트 클래스 경로 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	TSoftClassPtr<UAnimInstance> AnimBP = nullptr;
	/** 캐릭터 선택 UI에 표시할 초상화 텍스처 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|UI")
	TSoftObjectPtr<UTexture2D> Portrait = nullptr;
	
	/** 스테이지 진입 시 EquipmentComponent 첫 슬롯에 자동 장착할 무기 ID (DT_Weapon FK) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	FName DefaultWeaponID = NAME_None;

	/** 기본 최대 HP */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	float BaseHP = 100.f;
	/** 기본 이동 속도 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	float BaseMoveSpeed = 600.f;
	/** 기본 공격력 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	float BaseAttackPower = 10.f;

	/** 캐릭터 등급 — 그리드 정렬 기준 (SSR > SR > R > N) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|Unlock")
	ECharacterGrade Grade = ECharacterGrade::N;
	/** 해금 조건 타입 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|Unlock")
	ECharacterUnlockType UnlockType = ECharacterUnlockType::DEFAULT;
	/** UnlockType==STAGE_CLEAR 시 클리어해야 하는 스테이지 ID (DT_Stage FK) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|Unlock")
	FName UnlockStageID = NAME_None;
	/** UnlockType==CURRENCY 시 소모 재화량 (재화 시스템 미구현 — stub) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|Unlock")
	float UnlockCost = 0.f;
	
};

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
	/** 무기 베이스 타입(강화/진화용도) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	EWeaponBaseType BaseType;
	/** 무기 강화 레벨 (1~3). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	int32 WeaponLevel;
	/** DT_Skill FK — 이 무기가 사용하는 스킬 ID */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	FName SkillID;
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
// DT_Skill_Common_Static_Data — 스킬 기본 데이터 
// ----------------------------------------------------------------------------
class ABaseProjectile;

USTRUCT(BlueprintType)
struct FSkillCommonStaticData : public FTableRowBase
{
	GENERATED_BODY()

	/** PK */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	FName SkillID;
	/** DT_SkillEffect FK */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	FName SkillEffectID;
	/** DT_String FK — 스킬 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	FName SkillName;
	/** DT_String FK — 스킬 설명 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	FName Description;
	/** 스킬 태그 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	FGameplayTagContainer Tags;
	/** 스킬 쿨타임 (초). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	float Cooldown = 1.f;
	
};

// ----------------------------------------------------------------------------
// DT_Skill_Common_Resource_Data  (스킬 공통 리소스 데이터)
// ----------------------------------------------------------------------------
class UNiagaraSystem;
class USoundBase;

USTRUCT(BlueprintType)
struct FSkillCommonResourceData : public FTableRowBase
{
	GENERATED_BODY()
	
	/** PK */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	FName SkillID;
	/** GA 클래스 에셋 경로 (데이터 드리븐 로딩용) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	TSoftClassPtr<UGameplayAbility> GAClass;
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
	
	/** 투사체 액터 클래스 경로. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	TSoftClassPtr<ABaseProjectile> ProjectileClass;
	/** 소환형 액터 클래스 경로.*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Summon")
	TSoftClassPtr<AActor> SummonObjectClass;
	/** 소환형 프리뷰 액터 클래스 경로.*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Summon")
	TSoftClassPtr<AActor> SummonPreviewClass;
	
	/** 스킬 아이콘 텍스처 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	TSoftObjectPtr<UTexture2D> SkillIcon;
	
	/** 스킬 스폰 FX */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UNiagaraSystem> SpawnVFX;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> SpawnSFX;
	/** 스킬 비행 FX */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UNiagaraSystem> TrailVFX;
	/** 스킬 충돌 FX */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UNiagaraSystem> ImpactVFX;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> ImpactSFX;
};

// ----------------------------------------------------------------------------
// DT_Skill_Common_Params_Data — 스킬 공통 파라미터 데이터
// ----------------------------------------------------------------------------
USTRUCT(BlueprintType)
struct FSkillCommonParamData : public FTableRowBase
{
	GENERATED_BODY()
	
	/** PK */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SkillParam")
	FName SkillEffectID;
	/** DT_StatusEffect FK. 없으면 None */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SkillParam")
	FName StatusEffectID;
	/** 스킬 타입 (공격/방어/유틸) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SkillParam")
	ESkillType SkillType;
	/** 스킬 효과 발동 시간 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SkillParam")
	float Lifetime = 0.f;
	/** 스킬 사용 범위(m) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SkillParam")
	float Range = 0.f;
	
};


// ----------------------------------------------------------------------------
// DT_Skill_Attack_Common_Params_Data — 공격 스킬 공통 파라미터 데이터
// ----------------------------------------------------------------------------
USTRUCT(BlueprintType)
struct FSkillAttackCommonParamsData : public FTableRowBase
{
	GENERATED_BODY()

	/** PK */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SkillEffect")
	FName SkillEffectID;
	/** 소환 방식 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SkillEffect")
	ESpawnPattern SpawnType;
	/** 움직임 방식 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SkillEffect")
	EMoveType MoveType;
	/** 공격 방식 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SkillEffect")
	EHitType HitType;
	/** 투사체 이동 속도. 소환형은 0 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SkillEffect")
	float Speed = 0.f;
	/** 기본 데미지 or 효과 수치 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SkillEffect")
	float Amount = 0.f;
	
};


// ----------------------------------------------------------------------------
// DT_Skill_Attack_Spawn_Params_Data — 공격 스킬 소환 파라미터
// ----------------------------------------------------------------------------
USTRUCT(BlueprintType)
struct FSkillAttackSpawnParamsData : public FTableRowBase
{
	GENERATED_BODY()

	/** PK  */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	FName SkillEffectID;
	/** 생성 스킬 오브젝트 수 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	int32 SpawnCount = 1;
	/** 생성 소켓 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	FName SocketName;
	/** Spread 전용 — 부채꼴 전체 각도 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	float SpreadAngle = 0.f;
	
};

// ----------------------------------------------------------------------------
// DT_Skill_Attack_MoveType_Params_Homing — 우도형 전용 파라미터
// ----------------------------------------------------------------------------
USTRUCT(BlueprintType)
struct FSkillAttackMoveTypeParamsHoming : public FTableRowBase
{
	GENERATED_BODY()
	
	/** PK */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flight|Homing")
	FName SkillEffectID;
	/** 방향 보정 회전 속도 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flight|Homing")
	float TurnSpeed = 0.f;
	/** 유도 유효 범위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flight|Homing")
	float LockRange = 0.f;
	
};

// ----------------------------------------------------------------------------
// DT_Skill_Attack_MoveType_Params_Arc — 곡사형 전용 파라미터
// ----------------------------------------------------------------------------
USTRUCT(BlueprintType)
struct FSkillAttackMoveTypeParamsArc : public FTableRowBase
{
	GENERATED_BODY()

	/** PK */
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
	
};


// ----------------------------------------------------------------------------
// Skill_Attack_MoveType_Params_Summon — 소환형 전용 파라미터
// ----------------------------------------------------------------------------
USTRUCT(BlueprintType)
struct FSkillAttackMoveTypeParamsSummon : public FTableRowBase
{
	GENERATED_BODY()

	/** FK — Skill_Common_Params_Data 참조 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Summon")
	FName SkillEffectID;
	/** 자동 모드 타겟 탐색 범위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Summon")
	float SearchRange = 0.f;
	/** 장판 효과 범위 (선택. 0이면 미사용) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Summon")
	float SummonRadius = 0.f;
};

// ----------------------------------------------------------------------------
// DT_Skill_Attack_HitType_Params_Pierce — 관통형 전용 파라미터
// ----------------------------------------------------------------------------
USTRUCT(BlueprintType)
struct FSkillAttackHitTypeParamsPierce : public FTableRowBase
{
	GENERATED_BODY()

	/** PK */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flight|Pierce")
	FName SkillEffectID;
	/** 최대 관통 횟수 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flight|Pierce")
	int32 PierceCount = 1;  
	/** 관통마다 적용되는 데미지 감쇠율 (0.0~1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flight|Pierce")
	float DamageDecay = 0.f; 
	
};


// ----------------------------------------------------------------------------
// DT_Skill_Attack_HitType_Params_Area — 폭발형 전용 파라미터
// ----------------------------------------------------------------------------
USTRUCT(BlueprintType)
struct FSkillAttackHitTypeParamsArea : public FTableRowBase
{
	GENERATED_BODY()

	/** PK  */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flight|Explode")
	FName SkillEffectID;
	/** 타격 판정 반경 (직접 충돌용) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flight|Explode")
	float HitRadius;
	/** 거리 기반 데미지 감쇠 배율 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flight|Explode")
	float ExplosionDamageMultiplier;
	
};


// ----------------------------------------------------------------------------
// DT_Skill_Defense_Common_Params_Data — 방어 스킬 공통 파라미터 데이터
// ----------------------------------------------------------------------------
USTRUCT(BlueprintType)
struct FSkillDefenseCommonParamsData : public FTableRowBase
{
	GENERATED_BODY()
	
	/** PK  */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Defense")
	FName SkillEffectID;
	/** 방어타입(무적형/아머형)  */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Defense")
	EDefenseType DefenseType;
	/** 방어 어트리뷰트에 적용할 추가 수치  */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Defense")
	float ArmorAmount;
	/** 데미지 경감 비율 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Defense")
	float ArmorMultiplier;
	
};

// ----------------------------------------------------------------------------
// DT_Status_Effect_Data — 상태효과 정적 데이터
// ----------------------------------------------------------------------------
USTRUCT(BlueprintType)
struct FStatusEffectData : public FTableRowBase
{
	GENERATED_BODY()
	
	/** PK  */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Status Effect")
	FName StatusEffectID;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Status Effect")
	FName StatusName;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Status Effect")
	FName Description;
	//상태 아이콘
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Status Effect")
	TSoftObjectPtr<UTexture2D> StatusIcon;
	
};



// ----------------------------------------------------------------------------
// DT_Enemy — 에너미 기본 데이터
// ----------------------------------------------------------------------------
class AEnemyBaseCharacter;

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

	/** 스폰할 BP 에너미 클래스 에셋 경로 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	TSoftClassPtr<AEnemyBaseCharacter> EnemyClass;
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

	/** 노드맵에 표시할 스테이지 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stage|Display")
	FText DisplayName = FText::GetEmpty();

	/** 순차 해금 체인 — 이 스테이지를 클리어하면 해금되는 다음 스테이지 ID (없으면 체인 끝) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stage|Unlock")
	FName NextStageID = NAME_None;

	/** true이면 보스 노드 아이콘으로 표시 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stage|Display")
	bool bIsBoss = false;

	/** 이 스테이지를 해금하기 위해 클리어해야 하는 직전 스테이지 ID (없으면 항상 진입 가능 — 첫 스테이지) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stage|Unlock")
	FName UnlockStageID = NAME_None;

	/** 이 스테이지가 속한 UE 레벨 (OpenNextLevelByName 대상) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stage|Level")
	ELevelName WorldLevel = ELevelName::STAGE_1;

	/** 노드맵 썸네일 이미지 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stage|Display")
	TSoftObjectPtr<UTexture2D> Thumbnail = nullptr;
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


