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
