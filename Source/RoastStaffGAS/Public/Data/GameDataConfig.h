// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "GameDataConfig.generated.h"

/**
 * GDS가 로드할 DataTable 에셋 경로를 보관하는 DataAsset.
 * 에디터에서 경로를 세팅하면 코드 수정 없이 테이블 교체 가능.
 */
UCLASS()
class ROASTSTAFFGAS_API UGameDataConfig : public UDataAsset
{
	GENERATED_BODY()
	
public:
	//Character
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	TSoftObjectPtr<UDataTable> CharacterStaticTable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level")
	TSoftObjectPtr<UCurveTable> BaseStatCurveTable;
	
	//Weapon
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	TSoftObjectPtr<UDataTable> WeaponStaticTable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	TSoftObjectPtr<UCurveTable> WeaponDamageCurveTable;
	
	//Skill
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	TSoftObjectPtr<UDataTable> SkillCommonStaticTable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	TSoftObjectPtr<UDataTable> SkillCommonResourceTable;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	TSoftObjectPtr<UDataTable> SkillCommonParamsTable;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Attack")
	TSoftObjectPtr<UDataTable> SkillAttackCommonParamsTable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Attack")
	TSoftObjectPtr<UDataTable> SkillAttackSpawnParamsTable;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Attack|HitType")
	TSoftObjectPtr<UDataTable> SkillAttackHitTypeParamsPierceTable;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Attack|HitType")
	TSoftObjectPtr<UDataTable> SkillAttackHitTypeParamsAreaTable;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Attack|MoveType")
	TSoftObjectPtr<UDataTable> SkillAttackMoveTypeParamsArcTable;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Attack|MoveType")
	TSoftObjectPtr<UDataTable> SkillAttackMoveTypeParamsHomingTable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Attack|MoveType")
	TSoftObjectPtr<UDataTable> SkillAttackMoveTypeParamsSummonTable;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Defense")
	TSoftObjectPtr<UDataTable> SkillDefenseCommonParamsTable;
	
	//Status Effect
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	TSoftObjectPtr<UDataTable> StatusEffectStaticTable;
	
	//Enemy / Stage / Wave
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	TSoftObjectPtr<UDataTable> EnemyTable;

	/** DT_EnemyExtData — Ranged / Elite / Boss 확장 수치 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	TSoftObjectPtr<UDataTable> EnemyExtTable;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stage")
	TSoftObjectPtr<UDataTable> StageTable;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stage")
	TSoftObjectPtr<UDataTable> WaveTable;
	
};
