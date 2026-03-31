// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Data/EnumUITypes.h"
#include "UIManagerSettings.generated.h"

class URSBaseWidget;

/**
 * UUIManagerSettings
 *
 * Project Settings > RoastStaff > UIManager Settings 에서 편집.
 * EUIID별 위젯 BP 클래스와 레이어를 매핑하여 OpenUIByID()가 런타임 코드 없이 동작하도록 함.
 *
 * UIClassMap  : EUIID → 위젯 BP 클래스 (Soft 참조 — 필요 시 LoadSynchronous)
 * UILayerMap  : EUIID → EUILayer (에디터에서 반드시 UIClassMap과 동기화)
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "UIManager Settings"))
class ROASTSTAFFGAS_API UUIManagerSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	// Project Settings 카테고리/섹션 이름
	virtual FName GetCategoryName() const override { return FName("RoastStaff"); }
	virtual FName GetSectionName()  const override { return FName("UIManager Settings"); }

	/** EUIID → 위젯 BP 클래스 매핑. 에디터에서 각 ID에 WBP를 할당할 것. */
	UPROPERTY(Config, EditAnywhere, Category = "UI Mapping")
	TMap<EUIID, TSoftClassPtr<URSBaseWidget>> UIClassMap;

	/** EUIID → 레이어 타입 매핑. UIClassMap과 반드시 동기화. */
	UPROPERTY(Config, EditAnywhere, Category = "UI Mapping")
	TMap<EUIID, EUILayer> UILayerMap;

	/** 단일 접근점 — GetDefault<>() 래퍼 */
	FORCEINLINE static const UUIManagerSettings* Get()
	{
		return GetDefault<UUIManagerSettings>();
	}
};
