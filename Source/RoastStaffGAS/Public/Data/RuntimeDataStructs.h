// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnumTypes.h"
#include "Abilities/GameplayAbility.h"
#include "RuntimeDataStructs.generated.h"

/**
 * 
 */
// ----------------------------------------------------------------------------
// FWeaponEquipData — 무기 장착 시 필요한 데이터를 한 번에 전달하는 구조체
// GDS의 GetWeaponEquipData()가 반환
// ----------------------------------------------------------------------------
USTRUCT(BlueprintType)
struct FWeaponEquipData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) 
	FName WeaponID;
	UPROPERTY(BlueprintReadOnly) 
	FName SkillID;
	UPROPERTY(BlueprintReadOnly) 
	FName SkillEffectID;
	UPROPERTY(BlueprintReadOnly) 
	EWeaponSlotType SlotType;

	// GA/투사체/GE 클래스 경로 (TSoftClassPtr — 장착 시점에 LoadSynchronous)
	UPROPERTY(BlueprintReadOnly) 
	TSoftClassPtr<UGameplayAbility> GAClass;
	UPROPERTY(BlueprintReadOnly) 
	TSoftClassPtr<AActor> ProjectileClass;
	UPROPERTY(BlueprintReadOnly) 
	TSoftClassPtr<UGameplayEffect> DamageGEClass;
	UPROPERTY(BlueprintReadOnly) 
	TSoftClassPtr<UGameplayEffect> StatusGEClass;

	// 쿨타임 (DT_SkillEffect에서 가져옴)
	UPROPERTY(BlueprintReadOnly) 
	float Cooldown;

	// 투사체 공통 파라미터 (GA가 사용)
	UPROPERTY(BlueprintReadOnly) 
	float Damage;
	UPROPERTY(BlueprintReadOnly) 
	float Speed;
	UPROPERTY(BlueprintReadOnly) 
	float Lifetime;
	UPROPERTY(BlueprintReadOnly) 
	ESkillType SkillType;
	UPROPERTY(BlueprintReadOnly) 
	EFlightType FlightType;
	UPROPERTY(BlueprintReadOnly) 
	EHitType HitType;
	UPROPERTY(BlueprintReadOnly) 
	EExpireCondition ExpireCondition;
};

// ----------------------------------------------------------------------------
// FRSSkillInitData — GA → 투사체로 전달되는 초기화 데이터.
// GA가 GDS에서 조회한 결과를 채워서 SpawnProjectiles에 전달
// ----------------------------------------------------------------------------

USTRUCT(BlueprintType)
struct FRSSkillInitData
{
	GENERATED_BODY()

	// 스킬 식별
	UPROPERTY() 
	FName SkillID;
	UPROPERTY() 
	FName SkillEffectID;

	// GE 클래스
	UPROPERTY() 
	TSubclassOf<UGameplayEffect> DamageGEClass;
	UPROPERTY() 
	TSubclassOf<UGameplayEffect> StatusGEClass;   // 없으면 nullptr

	// 발사자 ASC 
	UPROPERTY() 
	TObjectPtr<UAbilitySystemComponent> InstigatorASC;

	// 투사체 파라미터
	UPROPERTY() 
	float Damage;
	UPROPERTY() 
	float Speed;
	UPROPERTY() 
	float Lifetime;

	// 스폰 파라미터
	UPROPERTY() 
	ESpawnPattern SpawnPattern;
	UPROPERTY() 
	int32 ProjectileCount;
	UPROPERTY() 
	float SpreadAngle;
};


// -------------------------------------------------------------------------
// 슬롯 런타임 데이터 — EquipmentComponent가 직접 관리
// -------------------------------------------------------------------------
USTRUCT()
struct FWeaponSlotInstanceData
{
	GENERATED_BODY()

	// GDS에서 받아온 정적 데이터
	FWeaponEquipData EquipData;

	// 런타임 가변 상태
	FGameplayAbilitySpecHandle AbilitySpecHandle;
	
	int32 SlotIndex = -1;
	float CooldownRemaining; //자동발사시 쿨타임 관리
	bool bIsActive;
	
	FTimerHandle AutoFireTimerHandle;

	bool IsEmpty() const { return EquipData.WeaponID.IsNone(); }
};