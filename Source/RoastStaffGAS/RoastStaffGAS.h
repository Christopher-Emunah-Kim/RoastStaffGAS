// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

// ============================================
// GAS 공통 헤더(PCH) - 프로젝트 전역에서 사용(Pre-Compile Header)
// ============================================
// GAS 
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"

// AttributeSet 관련
#include "AttributeSet.h"

// GameplayEffect 관련
#include "GameplayEffect.h"
#include "GameplayEffectTypes.h"

// GameplayAbility 관련
#include "Abilities/GameplayAbility.h"

// GameplayTag 관련
#include "GameplayTagContainer.h"

// 유틸리티
#include "AbilitySystemBlueprintLibrary.h"



/** Main log category used across the project */
DECLARE_LOG_CATEGORY_EXTERN(LogRoastStaffGAS, Log, All);
