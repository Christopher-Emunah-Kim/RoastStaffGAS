// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/RSBaseWidget.h"
#include "RSLoadingWidget.generated.h"

class UProgressBar;
class UImage;

/**
 * URSLoadingWidget
 *
 * TRANSITION 레벨 전용 로딩 화면 위젯.
 *
 */
UCLASS()
class ROASTSTAFFGAS_API URSLoadingWidget : public URSBaseWidget
{
	GENERATED_BODY()

public:
	virtual void OpenUI()    override;
	virtual void CloseUI()   override;
	virtual void RefreshUI() override;

	void SetLoadingProgress(float InProgress);
	void FinishLoading();

private:
	UFUNCTION()
	void UpdateLoadingAnimation();

	// ── 바인딩 위젯 ─────────────────────────────────────────────────────────
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> Bar_Loading;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Img_Gear;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Img_Icon;

	// ── 설정값 ───────────────────────────────────────────────────────────────
	UPROPERTY(EditDefaultsOnly, Category = "RS|Loading|Animation")
	float TimerInterval = 0.05f;
	UPROPERTY(EditDefaultsOnly, Category = "RS|Loading|Animation")
	float RotationPerSecond = 180.f;
	UPROPERTY(EditDefaultsOnly, Category = "RS|Loading|Animation")
	float WobbleFrequency = 2.f;
	UPROPERTY(EditDefaultsOnly, Category = "RS|Loading|Animation")
	float WobbleAngle = 15.f;

	// ── 런타임 상태 ──────────────────────────────────────────────────────────
	FTimerHandle LoadingTimerHandle;

	float CurrentRotation = 0.f;
	float AnimationTime   = 0.f;
	float ElapsedTime     = 0.f;
	float Progress        = 0.f;
};
