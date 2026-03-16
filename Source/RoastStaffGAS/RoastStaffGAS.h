// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

// ============================================
// 공통 헤더(PCH) - 프로젝트 전역에서 사용(Pre-Compile Header)
// ============================================
// GAS 
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"

// AttributeSet 관련
#include "AttributeSet.h"

// GameplayEffect 관련
#include "GameplayEffect.h"
#include "GameplayEffectTypes.h"
#include "GameplayEffectExtension.h"

// GameplayAbility 관련
#include "Abilities/GameplayAbility.h"

// GameplayTag 관련
#include "GameplayTagContainer.h"

// 유틸리티
#include "AbilitySystemBlueprintLibrary.h"

//엔진 제공
#include "Algo/RandomShuffle.h"
#include "Engine/GameInstance.h"

//네트워크 멀티플레이 용 헤더
#include "Net/UnrealNetwork.h"

//로그 시스템
#include "System/LoggingSystem.h"

/** Main log category used across the project */
DECLARE_LOG_CATEGORY_EXTERN(LogRoastStaffGAS, Log, All);


// -------------------------------------------------------------------------
// GameInstance 서브시스템 접근 매크로
//
// 사용 예:
//   GET_GI_SUBSYSTEM(UEquipmentSubsystem, EquipSys);
//
// World가 없는 컨텍스트(UObject 등)에서는 GI를 직접 넘겨야함:
//   GET_GI_SUBSYSTEM_FROM(UGameDataSubsystem, GDS, GI);
// -------------------------------------------------------------------------
#define GET_GI(VarName)                                  \
UGameInstance* VarName = GetGameInstance();                     \
check(VarName);

#define GET_GI_SUBSYSTEM(SubsystemClass, VarName)                               \
GET_GI(_GI_##VarName)                                            \
SubsystemClass* VarName = _GI_##VarName->GetSubsystem<SubsystemClass>();    \
check(VarName);

#define GET_GI_SUBSYSTEM_FROM(SubsystemClass, VarName, GameInstanceRef)      \
check(GameInstanceRef);                                                  \
SubsystemClass* VarName = (GameInstanceRef)->GetSubsystem<SubsystemClass>(); \
check(VarName);