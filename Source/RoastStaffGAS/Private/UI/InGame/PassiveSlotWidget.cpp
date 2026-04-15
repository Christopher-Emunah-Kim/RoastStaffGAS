// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/InGame/PassiveSlotWidget.h"
#include "RoastStaffGAS.h"
#include "Subsystems/GameDataSubsystem.h"
#include "Data/DataTableStructs.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/TextBlock.h"
#include "System/LoggingSystem.h"

void UPassiveSlotWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	ensureMsgf(Btn_PassiveSlot,  TEXT("Btn_PassiveSlot BindWidget 누락"));
	ensureMsgf(Img_PassiveIcon,  TEXT("Img_PassiveIcon BindWidget 누락"));
	ensureMsgf(Ovl_Tooltip,      TEXT("Ovl_Tooltip BindWidget 누락"));
	ensureMsgf(Txt_PassiveName,  TEXT("Txt_PassiveName BindWidget 누락"));
	ensureMsgf(Txt_PassiveDesc,  TEXT("Txt_PassiveDesc BindWidget 누락"));

	if (Btn_PassiveSlot)
	{
		Btn_PassiveSlot->OnHovered.AddDynamic(this, &UPassiveSlotWidget::OnSlotHovered);
		Btn_PassiveSlot->OnUnhovered.AddDynamic(this, &UPassiveSlotWidget::OnSlotUnhovered);
	}

	if (Ovl_Tooltip)
	{
		Ovl_Tooltip->SetVisibility(ESlateVisibility::Hidden);
	}

	SetVisibility(ESlateVisibility::Hidden);
}

void UPassiveSlotWidget::UpdateSlot(FName PassiveID)
{
	if (PassiveID.IsNone())
	{
		ClearSlot();
		return;
	}

	GET_GI_SUBSYSTEM_FROM(UGameDataSubsystem, GDS, GetWorld()->GetGameInstance())

	FPassiveStaticData PassiveData;
	if (!GDS->GetPassiveData(PassiveID, PassiveData))
	{
		KHS_WARN("PassiveSlotWidget: PassiveID [%s] 데이터 없음", *PassiveID.ToString());
		ClearSlot();
		return;
	}

	if (!PassiveData.Icon.IsNull())
	{
		LoadedPassiveIcon = PassiveData.Icon.LoadSynchronous();
		if (LoadedPassiveIcon && Img_PassiveIcon)
		{
			Img_PassiveIcon->SetBrushFromTexture(LoadedPassiveIcon);
		}
	}

	if (Txt_PassiveName)
	{
		Txt_PassiveName->SetText(PassiveData.DisplayName);
	}

	if (Txt_PassiveDesc)
	{
		Txt_PassiveDesc->SetText(PassiveData.Description);
	}

	if (Ovl_Tooltip)
	{
		Ovl_Tooltip->SetVisibility(ESlateVisibility::Hidden);
	}

	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void UPassiveSlotWidget::ClearSlot()
{
	if (Ovl_Tooltip)
	{
		Ovl_Tooltip->SetVisibility(ESlateVisibility::Hidden);
	}

	SetVisibility(ESlateVisibility::Hidden);
}

void UPassiveSlotWidget::OnSlotHovered()
{
	if (Ovl_Tooltip)
	{
		Ovl_Tooltip->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
}

void UPassiveSlotWidget::OnSlotUnhovered()
{
	if (Ovl_Tooltip)
	{
		Ovl_Tooltip->SetVisibility(ESlateVisibility::Hidden);
	}
}
