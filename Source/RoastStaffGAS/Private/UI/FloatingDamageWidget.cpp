// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/FloatingDamageWidget.h"
#include "Components/TextBlock.h"
#include "Character/Player/RSPlayerController.h"
#include "System/LoggingSystem.h"

void UFloatingDamageWidget::PlayFloatAnimation(float Damage)
{
	if (!Txt_Amount)
	{
		KHS_WARN(TEXT("Txt_Amount is not bound in %s"), *GetName());
	}
	else
	{
		Txt_Amount->SetText(FText::AsNumber(static_cast<int32>(Damage)));
	}

	if (!FadeAnimation)
	{
		KHS_WARN(TEXT("FadeAnimation is not bound in %s. Returning to pool immediately."), *GetName());
		OnFadeAnimationFinished();
		return;
	}

	FWidgetAnimationDynamicEvent AnimFinishedEvent;
	AnimFinishedEvent.BindDynamic(this, &UFloatingDamageWidget::OnFadeAnimationFinished);
	BindToAnimationFinished(FadeAnimation, AnimFinishedEvent);

	PlayAnimation(FadeAnimation);
}

void UFloatingDamageWidget::OnFadeAnimationFinished()
{
	if (FadeAnimation)
	{
		UnbindAllFromAnimationFinished(FadeAnimation);
	}

	ARSPlayerController* PC = Cast<ARSPlayerController>(GetOwningPlayer());
	if (!PC)
	{
		// PC 소멸 시(레벨 전환 등) — 풀 반납 없이 뷰포트에서만 제거
		RemoveFromParent();
		return;
	}

	PC->ReturnFloatingDamageToPool(this);
}