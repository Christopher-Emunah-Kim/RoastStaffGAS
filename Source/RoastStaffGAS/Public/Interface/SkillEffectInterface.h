// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Data/RuntimeDataStructs.h"
#include "SkillEffectInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class USkillEffectInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * ISkillEffectInterface
 * SkillEffect Actor 공통 Init 계약.
 *
 * 구현 의무:
 *  - InitEffect()에서 모든 런타임 파라미터를 캐시하고 이펙트를 활성화할 것.
 *  - Pool에 반납(OnPoolDeactivate)할 때 캐시 초기화 포함.
 */
class ROASTSTAFFGAS_API ISkillEffectInterface
{
	GENERATED_BODY()
public:
	/**
	 * 스폰 직후 초기화 — 모든 런타임 파라미터를 수신하고 이펙트를 활성화.
	 * @param InitData  GA가 조립한 초기화 번들 (SkillGEClass, Amount, Radius, Duration, FX 포함)
	 */
	virtual void InitEffect(const FSkillEffectInitData& InitData) = 0;
};