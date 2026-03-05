// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Attributes/BaseAttributeSet.h"
#include "EnemyAttributeSet.generated.h"

/**
 * 에너미 전용 AttributeSet
 * — 이후 ATK 추가 예정.
 */
UCLASS()
class ROASTSTAFFGAS_API UEnemyAttributeSet : public UBaseAttributeSet
{
	GENERATED_BODY()
	
public:
	UEnemyAttributeSet();
};
