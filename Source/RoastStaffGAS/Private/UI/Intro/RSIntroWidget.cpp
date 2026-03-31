// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Intro/RSIntroWidget.h"
#include "RoastStaffGAS.h"
#include "Components/Image.h"
#include "TimerManager.h"
#include "Animation/WidgetAnimation.h"

void URSIntroWidget::OpenUI()
{
	Super::OpenUI();
	PlayIntroAnimation();
}

void URSIntroWidget::RefreshUI()
{
	Super::RefreshUI();

	if (Img_1) { Img_1->SetVisibility(ESlateVisibility::Hidden); }
	if (Img_2) { Img_2->SetVisibility(ESlateVisibility::Hidden); }
	if (Img_3) { Img_3->SetVisibility(ESlateVisibility::Hidden); }
}

void URSIntroWidget::PlayIntroAnimation()
{
	if (FadeAnim)
	{
		FWidgetAnimationDynamicEvent AnimFinishedDel;
		AnimFinishedDel.BindDynamic(this, &URSIntroWidget::OnFinishedIntroAnim);
		BindToAnimationEvent(FadeAnim, AnimFinishedDel, EWidgetAnimationEvent::Finished);

		PlayAnimation(FadeAnim);
	}
	else
	{
		KHS_INFO(TEXT("URSIntroWidget::PlayIntroAnimation — FadeAnim 미할당, 즉시 타이틀 전환"));
		OnFinishedIntroAnim();
	}
}

void URSIntroWidget::OnFinishedIntroAnim()
{
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle,
		[this]()
		{
			OnTitleOpenRequestedDel.Broadcast();
		},
		0.5f,
		false
	);
}
