// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnumTypes.h"
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
class UNiagaraSystem;
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

