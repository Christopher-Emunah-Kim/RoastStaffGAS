// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ChargeGaugeWidget.generated.h"

class UProgressBar;

/**
 * UChargeGaugeWidget
 *
 * GA_CharacterSkill_Charge(스나이프) 전용 차징 게이지 오버레이.
 * - RSHUDWidget 자식으로 배치. ShowGauge/HideGauge로 Visibility 토글.
 * - NativeTick에서 경과 시간 기반 Fill 갱신.
 * - Ratio >= PerfectZoneThreshold(0.8) 진입 시 ProgressBar 색상 변경으로 퍼펙트 존 표시.
 */
UCLASS()
class ROASTSTAFFGAS_API UChargeGaugeWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** GA_CharacterSkill_Charge::StartCharging에서 호출 — 게이지 표시 + 타이머 시작 */
	void ShowGauge(float InMaxChargeTime);
	/** GA_CharacterSkill_Charge::CleanupCharging에서 호출 — 게이지 숨김 + 타이머 리셋 */
	void HideGauge();

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	/** Ratio에 따라 ProgressBar Fill + TintColor 갱신 */
	void UpdateVisual(float Ratio);

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> PBar_ChargeGauge;
	
	/** 퍼펙트 존(Ratio >= 0.8) 시 ProgressBar에 적용할 색상 */
	UPROPERTY(EditDefaultsOnly, Category = "MY|ChargeGauge")
	FLinearColor NormalColor = FLinearColor(0.2f, 0.6f, 1.f, 1.f);
	/** 퍼펙트 존(Ratio >= 0.8) 시 ProgressBar에 적용할 색상 */
	UPROPERTY(EditDefaultsOnly, Category = "MY|ChargeGauge")
	FLinearColor PerfectZoneColor = FLinearColor(1.f, 0.85f, 0.f, 1.f);

	// ── 런타임 상태 ──────────────────────────────────────────────────────────
	float MaxChargeTime = 1.f;
	float ElapsedTime   = 0.f;
	bool  bGaugeActive  = false;

	/** 퍼펙트 존 진입 임계 (GA와 동일 값 80%) */
	static constexpr float PerfectZoneThreshold = 0.8f;
};
