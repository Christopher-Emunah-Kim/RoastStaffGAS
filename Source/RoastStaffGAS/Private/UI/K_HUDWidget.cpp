// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/K_HUDWidget.h"
#include "UI/K_StateBarWidget.h"

UK_HUDWidget::UK_HUDWidget()
{
	
}

void UK_HUDWidget::BindToASC(UAbilitySystemComponent* InASC)
{
	if (!ensureMsgf(WBP_StateBar, TEXT("[HUD] Invalid StateBar Widget")))
	{
		return;
	}
	
	WBP_StateBar->BindToASC(InASC);
}
