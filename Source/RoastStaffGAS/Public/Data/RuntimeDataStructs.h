// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnumTypes.h"
#include "DataTableStructs.h"
#include "Abilities/GameplayAbility.h"
#include "RuntimeDataStructs.generated.h"


// ============================================================================
// RuntimeDataStructs.h
// GDS 복합 조회 인터페이스가 반환하는 런타임 번들 구조체 문서
//
// 구조:
//   FWeaponSlotEquipData     — 장착/슬롯/UI       ← GDS.GetWeaponSlotEquipData()
//   FSkillExecutionData      — GA 발동 파라미터   ← GDS.GetSkillExecutionData()
//   FSkillFXData             — 연출 데이터        ← GDS.GetSkillFXData()
//   FRSProjectileInitData    — GA→투사체 전달     ← GA 내부에서 직접 조립
//   FWeaponSlotInstanceData  — 슬롯 런타임 상태  ← EquipmentSubsystem 내부
// ============================================================================

class ABaseProjectile;
class AGroundEffectActor;
class UNiagaraSystem;
class ASummonPreviewObject;
class USoundBase;
class UTexture2D;

// ----------------------------------------------------------------------------
// FWeaponSlotEquipData — 장착/슬롯/UI에 필요한 데이터
// GDS.GetWeaponSlotEquipData(WeaponID) 반환
// ----------------------------------------------------------------------------

USTRUCT(BlueprintType)
struct FWeaponSlotEquipData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) 
	FName WeaponID;
	UPROPERTY(BlueprintReadOnly) 
	FName WeaponName;
	UPROPERTY(BlueprintReadOnly) 
	FName SkillID;
	UPROPERTY(BlueprintReadOnly) 
	FName SkillEffectID;
	
	UPROPERTY(BlueprintReadOnly) 
	ESkillType SkillType;
	UPROPERTY(BlueprintReadOnly)
	EMoveType MoveType;
	UPROPERTY(BlueprintReadOnly) 
	float Cooldown;
	
	UPROPERTY(BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> SkillIcon;
	
	/** GiveAbility 등록용 — 장착 시 1회 LoadSynchronous */
	UPROPERTY(BlueprintReadOnly)                                                                               
	TSoftClassPtr<UGameplayAbility> GAClass;

};

// ----------------------------------------------------------------------------                                  
// FSkillExecutionData — GA 발동 시 필요한 전체 파라미터                                                       
// GDS.GetSkillExecutionData(SkillID) 반환
// ---------------------------------------------------------------------------- 

USTRUCT(BlueprintType)
struct FSkillExecutionData
{
	GENERATED_BODY()

	// 스킬 식별
	UPROPERTY(BlueprintReadOnly) 
	FName SkillID;
	UPROPERTY(BlueprintReadOnly) 
	FName SkillEffectID;

	// 클래스 경로 (SoftClassPtr — GA에서 LoadSynchronous)                                                     
    UPROPERTY(BlueprintReadOnly)                                                                               
    TSoftClassPtr<UGameplayEffect> DamageGEClass;                                                              
    UPROPERTY(BlueprintReadOnly)                                                                               
    TSoftClassPtr<UGameplayEffect> StatusGEClass;      // 없으면 null                                          
    UPROPERTY(BlueprintReadOnly)                                                                               
    TSoftClassPtr<ABaseProjectile> ProjectileClass;    // 투사체형                                             
    UPROPERTY(BlueprintReadOnly)                                                                               
    TSoftClassPtr<AActor> SummonObjectClass;           // 소환형                                              
    UPROPERTY(BlueprintReadOnly)                                                                               
    TSoftClassPtr<AActor> SummonPreviewClass;          // 소환형 프리뷰                                       
                                                                                                               
    // 스킬 타입 분류                                                                                        
    UPROPERTY(BlueprintReadOnly)                                                                               
    ESkillType SkillType;                                                               
    UPROPERTY(BlueprintReadOnly)                                                                               
    EMoveType MoveType;                                                                    
    UPROPERTY(BlueprintReadOnly)                                                                               
    EHitType HitType;                                                                       
    UPROPERTY(BlueprintReadOnly)                                                                             
    ESpawnPattern SpawnPattern;                                                        
                                                                                                               
    // 수치 파라미터                                                                                           
    UPROPERTY(BlueprintReadOnly)                                                                               
    float Amount;                                                                                      
    UPROPERTY(BlueprintReadOnly)                                                                               
    float Speed;                                                                                         
    UPROPERTY(BlueprintReadOnly)                                                                               
    float Lifetime;                                                                                      
    UPROPERTY(BlueprintReadOnly)                                                                             
    float Range;
                                                                                                               
    // 스폰 파라미터
    UPROPERTY(BlueprintReadOnly)                                                                               
    int32 SpawnCount;                                                                                    
    UPROPERTY(BlueprintReadOnly)
    FName SocketName;                                                                                          
    UPROPERTY(BlueprintReadOnly)
    float SpreadAngle;                 
};

// ----------------------------------------------------------------------------                                  
// FSkillFXData — 연출 데이터                                                                                  
// GDS.GetSkillFXData(SkillID) 반환
// SkillCommonResource에서 FX 필드만 추출                                                             
// ----------------------------------------------------------------------------
USTRUCT(BlueprintType)                                                                                           
struct FSkillFXData                                                                                              
{                                                                                                                
    GENERATED_BODY()                                                                                           
                                                                                                               
    UPROPERTY(BlueprintReadOnly)                                                                             
    TSoftObjectPtr<UNiagaraSystem> SpawnVFX;
    UPROPERTY(BlueprintReadOnly)
    TSoftObjectPtr<USoundBase> SpawnSFX;                                                                       
    UPROPERTY(BlueprintReadOnly)
    TSoftObjectPtr<UNiagaraSystem> TrailVFX;                                                                   
    UPROPERTY(BlueprintReadOnly)                                                                             
    TSoftObjectPtr<UNiagaraSystem> ImpactVFX;                                                                  
    UPROPERTY(BlueprintReadOnly)                                                                               
    TSoftObjectPtr<USoundBase> ImpactSFX;                                                                      
};                                                                                                               
                                                                                                                 
// ----------------------------------------------------------------------------
// FProjectileInitData — GA → 투사체 전달 데이터 
// GA가 FSkillExecutionData의 SoftClassPtr을 로드 완료 후 직접 조립                                              
// SoftClassPtr → TSubclassOf 변환 완료 상태                                                                     
// ----------------------------------------------------------------------------   
class USceneComponent;
USTRUCT(BlueprintType)                                                                                           
struct FProjectileInitData                                                                                     
{
    GENERATED_BODY()                                                                                           
                                                                                                             
    UPROPERTY()
    FName SkillID;
    UPROPERTY()
    FName SkillEffectID;
    UPROPERTY()
    TSubclassOf<UGameplayEffect> DamageGEClass;
    UPROPERTY()                                                                                                
    TSubclassOf<UGameplayEffect> StatusGEClass;     // 없으면 nullptr
    UPROPERTY()                                                                                                
    TObjectPtr<UAbilitySystemComponent> InstigatorASC;                                                         
                                                                                                               
    UPROPERTY()                                                                                                
    float Amount;                                                                                        
    UPROPERTY()                                                                                              
    float Speed;
    UPROPERTY()
    float Lifetime;
                                                                                                               
    UPROPERTY()
    ESpawnPattern SpawnPattern;      
	UPROPERTY()
	EMoveType MoveType;
	UPROPERTY()
	EHitType HitType;
    UPROPERTY()                                                                                              
    int32 SpawnCount;
    UPROPERTY()
    float SpreadAngle;
	// 타입별 필요 정보
	// MoveType - Homing
	UPROPERTY()
    TWeakObjectPtr<USceneComponent> HomingTarget;
    UPROPERTY()
    float TurnSpeed;
    UPROPERTY()
    float LaunchAngle;
    UPROPERTY()
    float GravityScale;
    //HitType - Area
    UPROPERTY()
    float HitRadius;
    // HitType - Pierce
    UPROPERTY()
    int32 PierceCount;
    UPROPERTY()
    float DamageDecay;
};                                                                                                               
                                   
// ----------------------------------------------------------------------------
// FSummonObjectInitData — GA → 소환 오브젝트 전달 데이터                                                         
// GA가 FSkillExecutionData를 기반으로 직접 조립                                                                 
// ----------------------------------------------------------------------------                                  
USTRUCT(BlueprintType)                                                                                           
struct FSummonObjectInitData                                                                                     
{                                                                                                                
    GENERATED_BODY()                                                                                          
    UPROPERTY()
    FName SkillID;
                                                                                                                 
    UPROPERTY()
    TSubclassOf<UGameplayEffect> DamageGEClass;                                                                  
    UPROPERTY()                                                                                                  
    TSubclassOf<UGameplayEffect> StatusGEClass;     //선택                                                             
    UPROPERTY()                                                                                                  
    TObjectPtr<UAbilitySystemComponent> InstigatorASC;                                                         
                                                                                                                 
    UPROPERTY()
    float Amount;                                                                                          
    UPROPERTY()                                                                                                  
    float SummonRadius;                                                                                    
    UPROPERTY()
    float Lifetime;                                                                                        
};                                   


// ----------------------------------------------------------------------------
// FWeaponCardDisplayData — 레벨업 UI 무기 카드 표시 데이터
// LevelUpSubsystem → LevelUpWeaponSelectWidget 전달용
// ----------------------------------------------------------------------------
USTRUCT(BlueprintType)
struct FWeaponCardDisplayData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FName WeaponID;
	UPROPERTY(BlueprintReadOnly)
	FName WeaponName;
	UPROPERTY(BlueprintReadOnly)
	FName Description;
	UPROPERTY(BlueprintReadOnly)
	EWeaponCardState CardState = EWeaponCardState::New;
	/** DT_Combination 미구현 — 항상 false (스텁) */
	UPROPERTY(BlueprintReadOnly)
	bool bCanEvolve = false;
	/** 무기 카드 아이콘 — GDS.GetWeaponSlotEquipData().SkillIcon 경유 */
	UPROPERTY(BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> WeaponIcon;
};

// ----------------------------------------------------------------------------
// FCharacterSkillExecData — GDS 복합 조회 반환 (캐릭터 스킬 실행 번들)
// GDS.GetCharacterSkillExecData(CharacterID, SkillSlot, SkillLevel) 반환
// SkillManagerSubsystem이 GA 발동 시 사용
// ----------------------------------------------------------------------------
USTRUCT(BlueprintType)
struct FCharacterSkillExecData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FName SkillID;
	UPROPERTY(BlueprintReadOnly)
	ESkillActivationType ActivationType = ESkillActivationType::InstantAoE;
	UPROPERTY(BlueprintReadOnly)
	float Cooldown = 10.f;
	/** GA 클래스 (InitializeSkills 시 LoadSynchronous) */
	UPROPERTY(BlueprintReadOnly)
	TSoftClassPtr<UGameplayAbility> GAClass;
	/** SpawnPreview 타입 전용 프리뷰 액터 클래스. 다른 타입에서는 null */
	UPROPERTY(BlueprintReadOnly)
	TSoftClassPtr<ASummonPreviewObject> PreviewActorClass;
	/** 스킬 슬롯 아이콘 텍스처 */
	UPROPERTY(BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> SkillIconSoftRef;
	/** 해당 레벨 수치+FX 데이터 (FCharacterSkillStaticData.LevelData[Level-1]) */
	UPROPERTY(BlueprintReadOnly)
	FCharacterSkillLevelData LevelData;

	// ── ProjectileSpawn — SkillEffectID 복합 조회 결과 ─────────────────────
	// GDS.GetCharacterSkillExecData 내에서 SkillEffectID로 무기 스킬 테이블을 복합 조회하여 채운다.
	// SkillEffectID == NAME_None이면 아래 필드 미사용 (InstantAoE / SelfBuff / SpawnPreview만 사용).

	/** 속성 태그 — SpawnSkillFX ElementColor 분기용 (FCharacterSkillStaticData.ElementTag 그대로 전달) */
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag ElementTag;

	/** GroundEffect 전용 장판 Actor 클래스 (FCharacterSkillStaticData.GroundEffectActorClass 그대로 전달) */
	UPROPERTY(BlueprintReadOnly)
	TSoftClassPtr<AGroundEffectActor> GroundEffectActorClass;

	/** 스킬 효과 FK (FCharacterSkillStaticData.SkillEffectID 그대로 전달) */
	UPROPERTY(BlueprintReadOnly)
	FName SkillEffectID;

	/** 투사체 클래스 — SkillCommonResourceCache[SkillID].ProjectileClass */
	UPROPERTY(BlueprintReadOnly)
	TSoftClassPtr<ABaseProjectile> ProjectileClass;

	/** 기본 데미지 수치 — SkillAttackCommonParamsData.Amount.
	 *  FireOneProjectile에서 Amount * LevelData.DamageMultiplier 로 최종 데미지 산출. */
	UPROPERTY(BlueprintReadOnly)
	float Amount = 0.f;

	/** 투사체 이동 속도 (cm/s) — SkillAttackCommonParamsData.Speed */
	UPROPERTY(BlueprintReadOnly)
	float ProjectileSpeed = 0.f;

	/** 투사체 수명 (초) — SkillCommonParamData.Lifetime */
	UPROPERTY(BlueprintReadOnly)
	float ProjectileLifetime = 0.f;

	/** 이동 방식 — SkillAttackCommonParamsData.MoveType */
	UPROPERTY(BlueprintReadOnly)
	EMoveType MoveType = EMoveType::LINEAR;

	/** 타격 방식 — SkillAttackCommonParamsData.HitType */
	UPROPERTY(BlueprintReadOnly)
	EHitType HitType = EHitType::SINGLE;

	/** 소환 방식 — SkillAttackCommonParamsData.SpawnType */
	UPROPERTY(BlueprintReadOnly)
	ESpawnPattern SpawnPattern = ESpawnPattern::SINGLE;

	/** 연속 발사 수 — SkillAttackSpawnParamsData.SpawnCount */
	UPROPERTY(BlueprintReadOnly)
	int32 ProjectileCount = 1;

	/** 연속 발사 간격 (초) — FCharacterSkillStaticData.FireInterval (캐릭터 스킬 전용) */
	UPROPERTY(BlueprintReadOnly)
	float FireInterval = 0.f;

	/** 관통 횟수 — SkillHitTypePierceCache.PierceCount (0 = 비관통) */
	UPROPERTY(BlueprintReadOnly)
	int32 PierceCount = 0;

	/** 관통 데미지 감쇠율 — SkillHitTypePierceCache.DamageDecay */
	UPROPERTY(BlueprintReadOnly)
	float DamageDecay = 0.f;
};

// ----------------------------------------------------------------------------
// FLevelUpCardDisplayData — 레벨업 UI 카드 표시 데이터
// LevelUpSubsystem → LevelUpWeaponSelectWidget 전달용
// 정적 카드(DT_LevelUpCard) + 동적 카드(무기 업그레이드/신규) 통합
// ----------------------------------------------------------------------------
USTRUCT(BlueprintType)
struct FLevelUpCardDisplayData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FName CardID;
	UPROPERTY(BlueprintReadOnly)
	ELevelUpCardType CardType = ELevelUpCardType::StatUpgrade;
	UPROPERTY(BlueprintReadOnly)
	FText DisplayName = FText::GetEmpty();
	UPROPERTY(BlueprintReadOnly)
	FText Description = FText::GetEmpty();
	/** 카드 아이콘 — 무기 카드: SkillIcon, 패시브 카드: Passive.Icon, 스탯 카드: 별도 UI 에셋 */
	UPROPERTY(BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> Icon;
	/** 가중 랜덤 샘플링용 가중치 */
	UPROPERTY(BlueprintReadOnly)
	float Weight = 1.f;
};

// ----------------------------------------------------------------------------
// FCharacterPreloadBundle — 트랜지션 로딩 캐릭터 에셋 프리로드 묶음
// GDS.GetCharacterPreloadBundle(CharID) 반환
// ----------------------------------------------------------------------------
class USkeletalMesh;
class UAnimInstance;

USTRUCT()
struct FCharacterPreloadBundle
{
	GENERATED_BODY()

	UPROPERTY()
	TSoftObjectPtr<USkeletalMesh>  Mesh;
	UPROPERTY()
	TSoftClassPtr<UAnimInstance>   AnimBP;
	// 추후 추가: IdleVFX, PortraitUI 등
};

// ----------------------------------------------------------------------------
// FEnemyPreloadBundle — 트랜지션 로딩 에너미 에셋 프리로드 묶음
// GDS.GetEnemyPreloadBundle(EnemyID) 반환
// ----------------------------------------------------------------------------
class AEnemyBaseCharacter;
class UBehaviorTree;

USTRUCT()
struct FEnemyPreloadBundle
{
	GENERATED_BODY()

	UPROPERTY()
	TSoftClassPtr<AEnemyBaseCharacter> EnemyClass;
	UPROPERTY()
	TSoftObjectPtr<UBehaviorTree>      BehaviorTree;
};

// ----------------------------------------------------------------------------
// FWeaponSlotInstanceData — 슬롯 런타임 상태
// EquipmentSubsystem이 직접 관리
// ----------------------------------------------------------------------------
USTRUCT()
struct FWeaponSlotInstanceData
{
    GENERATED_BODY()
	/** GDS에서 받아온 장착 데이터 */
    FWeaponSlotEquipData SlotEquipData;

    /** GA 핸들 */
    FGameplayAbilitySpecHandle AbilitySpecHandle;

    int32 SlotIndex;
    float CooldownRemaining;
    bool bIsActive;

    FTimerHandle AutoFireTimerHandle;

    bool IsEmpty() const { return SlotEquipData.WeaponID.IsNone(); }
};

// ----------------------------------------------------------------------------
// FStageResultData — 스테이지 종료 시 결과 데이터
// RSGameMode → SaveGameSubsystem::UpdateStageRecord() 전달용
// ----------------------------------------------------------------------------
USTRUCT(BlueprintType)
struct FStageResultData
{
	GENERATED_BODY()

	/** 플레이 시간 (초) */
	UPROPERTY(BlueprintReadOnly)
	float SurvivalTime = 0.f;
	/** 처치한 적 수 */
	UPROPERTY(BlueprintReadOnly)
	int32 KillCount = 0;
	/** 클리어 여부 (true: 타임오버 생존, false: 플레이어 사망) */
	UPROPERTY(BlueprintReadOnly)
	bool bCleared = false;
};                                                                                                             

