// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Engine/StreamableManager.h"
#include "RSTransitionGameMode.generated.h"

class URSLoadingWidget;

/**
 * ARSTransitionGameMode
 *
 * TRANSITION 레벨 전용 GameMode.
 * 에셋 프리로드 + 레벨 스트리밍 완료 후 Stage 레벨로 즉시 전환한다.
 * LoadingWidget 닫힘은 Stage 레벨의 PreWarm 완료 시점에 처리된다.
 */
UCLASS()
class ROASTSTAFFGAS_API ARSTransitionGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ARSTransitionGameMode();

	virtual void BeginPlay() override;

private:
	/** 에셋 프리로드 진입점. RDS::GatherPreloadAssets → FStreamableManager 비동기 로드 → StartLevelStreaming */
	void PreloadAssetsAsync();
	/** MapSettings::LevelMap[NextLevelName] 경로로 LoadStreamLevel 요청 */
	void StartLevelStreaming();
	/** LoadStreamLevel 완료 콜백 — 즉시 Stage 레벨로 전환 */
	UFUNCTION()
	void OnLevelPreloadCompleted();

private:
	/** 로딩 진행률 표시 위젯 (PAGE 레이어) */
	UPROPERTY()
	TObjectPtr<URSLoadingWidget> LoadingWidget;

	/** 비동기 프리로드 핸들 — GC 방지용 레퍼런스 유지 */
	TSharedPtr<FStreamableHandle> StreamableHandle;
};
