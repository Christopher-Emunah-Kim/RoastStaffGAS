// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/RSHUDWidget.h"
#include "UI/WeaponSlotContainerWidget.h"

URSHUDWidget::URSHUDWidget()
{
}

void URSHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
	ensureMsgf(WBP_SlotContainer, TEXT("SlotContainerWidget BindWidget 누락"));

}
