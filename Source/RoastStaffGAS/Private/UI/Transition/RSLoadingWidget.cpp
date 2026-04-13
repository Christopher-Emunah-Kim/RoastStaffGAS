// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Transition/RSLoadingWidget.h"
#include "RoastStaffGAS.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"
#include "TimerManager.h"

void URSLoadingWidget::OpenUI()
{
	Super::OpenUI();

	GetWorld()->GetTimerManager().SetTimer(
		LoadingTimerHandle,	this,	&URSLoadingWidget::UpdateLoadingAnimation,
		TimerInterval,true	);
}

void URSLoadingWidget::CloseUI()
{
	Super::CloseUI();

	GetWorld()->GetTimerManager().ClearTimer(LoadingTimerHandle);
}

void URSLoadingWidget::RefreshUI()
{
	Super::RefreshUI();

	CurrentRotation = 0.f;
	AnimationTime   = 0.f;
	ElapsedTime     = 0.f;
	Progress        = 0.f;

	if (Bar_Loading)
	{
		Bar_Loading->SetPercent(Progress);
	}

	GetWorld()->GetTimerManager().ClearTimer(LoadingTimerHandle);
}

void URSLoadingWidget::UpdateLoadingAnimation()
{
	AnimationTime += TimerInterval;

	if (Img_Gear)
	{
		CurrentRotation += RotationPerSecond * TimerInterval;
		if (CurrentRotation >= 360.f)
		{
			CurrentRotation -= 360.f;
		}
		Img_Gear->SetRenderTransformAngle(CurrentRotation);
	}

	if (Img_Icon)
	{
		float CurrentWobbleAngle = FMath::Sin(AnimationTime * WobbleFrequency) * WobbleAngle;
		Img_Icon->SetRenderTransformAngle(CurrentWobbleAngle);
	}
}

void URSLoadingWidget::SetLoadingProgress(float InProgress)
{
	Progress = FMath::Clamp(InProgress, 0.f, 1.f);
	if (Bar_Loading)
	{
		Bar_Loading->SetPercent(Progress);
	}
}

void URSLoadingWidget::FinishLoading()
{
	if (!IsVisible())
	{
		KHS_WARN(TEXT("LoadingWidget이 이미 안 보임 — bIsOpen: %d, IsInViewport: %d"), IsOpen(), IsInViewport());
		return;
	}
	SetLoadingProgress(1.f);
}
