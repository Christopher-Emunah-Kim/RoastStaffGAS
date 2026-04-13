// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "RSCharacterSkillData.generated.h"

/**
 * URSCharacterSkillData
 * GA_CharacterSkill 부여 시 SkillSlot 정보를 SourceObject로 전달하는 경량 UObject.
 * SkillManagerSubsystem이 GiveAbility 시 생성하여 Spec에 주입.
 * GA는 GetCurrentSourceObject()로 꺼내 SkillManagerSubsystem::GetSlotExecData(SlotIndex) 호출.
 */
UCLASS()
class ROASTSTAFFGAS_API URSCharacterSkillData : public UObject
{
	GENERATED_BODY()

public:
	/** 스킬 슬롯 인덱스 (0=Q/Slot1, 1=E/Slot2) */
	UPROPERTY()
	int32 SlotIndex = 0;
	/** 스킬 ID (로깅 및 디버그 용도) */
	UPROPERTY()
	FName SkillID;
};
