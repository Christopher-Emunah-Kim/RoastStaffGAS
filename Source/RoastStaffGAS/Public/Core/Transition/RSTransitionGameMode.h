// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "RSTransitionGameMode.generated.h"

class URSLoadingWidget;

/**
 * ARSTransitionGameMode
 *
 * TRANSITION 레벨 전용 GameMode.
 * 에셋 프리로드(stub) + FakeProgress 보간 + 레벨 스트리밍을 담당한다.
 *
 * 흐름:
 *   BeginPlay → PreloadAssetsAsync()
 *     └→ StartLevelStreaming()
 *          └→ OnLevelPreloadCompleted() 콜백
 *               ├→ bIsLoadingLevel = false
 *               ├→ LoadingWidget::FinishLoading()
 *               └→ SetTimer(1.0s) → GI::OpenNextLevelLatent()
 *
 * Tick(): bIsLoadingLevel 동안 FakeProgress 0→0.9 보간 → LoadingWidget::SetLoadingProgress()
 *
 * BP 설정: TRANSITION 레벨 World Settings → GameMode Override → BP_RSTransitionGameMode
 */
UCLASS()
class ROASTSTAFFGAS_API ARSTransitionGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ARSTransitionGameMode();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	/**
	 * 에셋 프리로드 진입점.
	 * TODO(PLAN_Data MODULE-2): RuntimeDS::GatherPreloadAssets(OutPaths) 호출로 교체 예정.
	 * 현재는 에셋 수집 없이 바로 레벨 스트리밍 진행.
	 */
	void PreloadAssetsAsync();

	/** MapSettings::LevelMap[NextLevelName] 경로로 LoadStreamLevel 요청 */
	void StartLevelStreaming();

	/** LoadStreamLevel 완료 콜백 */
	UFUNCTION()
	void OnLevelPreloadCompleted();

	/** 최종 레벨 이동 타이머 콜백 */
	void OpenNextLevel();

	/** 로딩 진행률 표시 위젯 (PAGE 레이어) */
	UPROPERTY()
	TObjectPtr<URSLoadingWidget> LoadingWidget;

	bool  bIsLoadingLevel    = false;
	float CurrentFakeProgress = 0.f;
};
