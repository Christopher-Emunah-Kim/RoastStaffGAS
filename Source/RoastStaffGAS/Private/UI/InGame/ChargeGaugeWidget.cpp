// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InGame/ChargeGaugeWidget.h"
#include "Components/ProgressBar.h"
#include "System/LoggingSystem.h"

void UChargeGaugeWidget::ShowGauge(float InMaxChargeTime)
{
    if (!ensureMsgf(PBar_ChargeGauge, TEXT("ChargeGaugeWidget: PBar_ChargeGauge BindWidget 누락")))
    {
        return;
    }

    MaxChargeTime = FMath::Max(InMaxChargeTime, KINDA_SMALL_NUMBER);
    ElapsedTime   = 0.f;
    bGaugeActive  = true;

    UpdateVisual(0.f);
    SetVisibility(ESlateVisibility::Visible);
}

void UChargeGaugeWidget::HideGauge()
{
    bGaugeActive = false;
    ElapsedTime  = 0.f;
    SetVisibility(ESlateVisibility::Collapsed);
}

void UChargeGaugeWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (!bGaugeActive)
    {
        return;
    }

    ElapsedTime += InDeltaTime;
    // MaxChargeTime 초과 시 1.0으로 고정 (타임아웃 자동발사 전 시각 피드백 유지)
    const float Ratio = FMath::Clamp(ElapsedTime / MaxChargeTime, 0.f, 1.f);
    UpdateVisual(Ratio);
}

void UChargeGaugeWidget::UpdateVisual(float Ratio)
{
    PBar_ChargeGauge->SetPercent(Ratio);

    const FLinearColor& TargetColor = (Ratio >= PerfectZoneThreshold) ? PerfectZoneColor : NormalColor;
    PBar_ChargeGauge->SetFillColorAndOpacity(TargetColor);
}
