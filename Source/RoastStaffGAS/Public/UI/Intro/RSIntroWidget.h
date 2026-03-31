// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/RSBaseWidget.h"
#include "RSIntroWidget.generated.h"

class UImage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTitleOpenRequested);

/**
 * URSIntroWidget
 *
 * INTRO 레벨 타이틀 전 페이드인 위젯.
 * - OpenUI          : FadeAnim 재생 시작
 * - OnFinishedIntroAnim : 0.5s 후 OnTitleOpenRequestedDel 브로드캐스트
 *
 * 바인딩:
 *   ARSIntroPlayerController::OpenFirstWidget()에서
 *   GetOrCreateWidgetByID(INTRO) → Cast → OnTitleOpenRequestedDel.AddUniqueDynamic
 */
UCLASS()
class ROASTSTAFFGAS_API URSIntroWidget : public URSBaseWidget
{
	GENERATED_BODY()

public:
	virtual void OpenUI()    override;
	virtual void RefreshUI() override;

	/** ARSIntroPlayerController::OpenTitleScreen에 바인딩 */
	UPROPERTY(BlueprintAssignable, Category = "RS|Intro")
	FOnTitleOpenRequested OnTitleOpenRequestedDel;

private:
	void PlayIntroAnimation();

	UFUNCTION()
	void OnFinishedIntroAnim();

	// ── 바인딩 위젯 ─────────────────────────────────────────────────────────
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Img_1;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Img_2;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Img_3;

	// ── 바인딩 애니메이션 ────────────────────────────────────────────────────
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> FadeAnim;
};
