// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Data/EnumUITypes.h"
#include "MapSettings.generated.h"

/**
 * UMapSettings
 *
 * Project Settings > RoastStaff > Map Settings 에서 편집.
 * ELevelName → 실제 레벨 에셋 경로 매핑.
 * RSGameInstance::OpenNextLevelLatent()가 이 맵을 참조하여 목적지 레벨을 결정한다.
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Map Settings"))
class ROASTSTAFFGAS_API UMapSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetCategoryName() const override { return FName("RoastStaff"); }
	virtual FName GetSectionName()  const override { return FName("Map Settings"); }

	
	FORCEINLINE static const UMapSettings* Get()
	{
		return GetDefault<UMapSettings>();
	}
	
	FORCEINLINE const TSoftObjectPtr<UWorld>* GetWorldMapBy(ELevelName LevelName) const
	{
		return LevelMap.Find(LevelName);
	}
	
private:
	/** ELevelName → 레벨 에셋 경로 매핑. 에디터에서 각 항목에 레벨 에셋을 할당. */
	UPROPERTY(Config, EditAnywhere, Category = "Level Map")
	TMap<ELevelName, TSoftObjectPtr<UWorld>> LevelMap;
};
