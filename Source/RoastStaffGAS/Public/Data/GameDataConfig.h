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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level")
	TSoftObjectPtr<UCurveTable> BaseStatCurveTable;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	TSoftObjectPtr<UDataTable> WeaponTable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	TSoftObjectPtr<UDataTable> SkillTable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	TSoftObjectPtr<UDataTable> SkillEffectTable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	TSoftObjectPtr<UDataTable> SpawnTable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Flight")
	TSoftObjectPtr<UDataTable> FlightArcTable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Flight")
	TSoftObjectPtr<UDataTable> FlightPierceTable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Flight")
	TSoftObjectPtr<UDataTable> FlightExplodeTable;
	
	
};
