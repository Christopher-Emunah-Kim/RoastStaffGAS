// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "RSSkillData.generated.h"

/**
 * URSSkillData
 * GA 부여 시점에 SkillID를 SourceObject로 전달하기 위한 경량 UObject.
 * EquipmentComponent가 GiveAbility 시 생성하여 Spec에 주입
 * GA는 GetCurrentSourceObject()로 이 객체를 꺼내 SkillID를 참조
 */
UCLASS()
class ROASTSTAFFGAS_API URSSkillData : public UObject
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	FName SkillID;
	UPROPERTY()
	FName WeaponID;
};
