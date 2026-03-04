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