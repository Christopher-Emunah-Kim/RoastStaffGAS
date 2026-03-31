// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Intro/RSTitleWidget.h"
#include "Components/Button.h"

void URSTitleWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (Btn_Start)
	{
		Btn_Start->OnClicked.AddDynamic(this, &URSTitleWidget::HandleStartClicked);
	}
}

void URSTitleWidget::HandleStartClicked()
{
	OnStartGameRequestedDel.Broadcast();
}
