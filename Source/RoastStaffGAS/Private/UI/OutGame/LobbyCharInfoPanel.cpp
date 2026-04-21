// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/OutGame/LobbyCharInfoPanel.h"
#include "Core/OutGame/RSOutGamePlayerController.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Button.h"
#include "Subsystems/GameDataSubsystem.h"
#include "Data/DataTableStructs.h"
#include "RoastStaffGAS.h"
#include "System/LoggingSystem.h"

void ULobbyCharInfoPanel::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (Btn_Confirm)
	{
		Btn_Confirm->OnClicked.AddUniqueDynamic(this, &ThisClass::OnConfirmButtonClicked);
	}

	if (Btn_Back)
	{
		Btn_Back->OnClicked.AddUniqueDynamic(this, &ThisClass::OnBackButtonClicked);
	}
}

void ULobbyCharInfoPanel::Show(FName CharID)
{
	SetVisibility(ESlateVisibility::Visible);
	PopulateWithCharacter(CharID);
}

void ULobbyCharInfoPanel::Hide()
{
	SetVisibility(ESlateVisibility::Collapsed);
}

void ULobbyCharInfoPanel::PopulateWithCharacter(FName CharID)
{
	GET_GI_SUBSYSTEM(UGameDataSubsystem, GDS)

	FCharacterStaticData CharData;
	if (!GDS->GetCharacterStaticData(CharID, CharData))
	{
		KHS_WARN("CharID 데이터 없음: %s", *CharID.ToString());
		return;
	}

	if (Txt_CharName)
	{
		Txt_CharName->SetText(CharData.DisplayName);
	}

	if (Img_Portrait && !CharData.Portrait.IsNull())
	{
		UTexture2D* PortraitTex = CharData.Portrait.LoadSynchronous();
		if (PortraitTex)
		{
			Img_Portrait->SetBrushFromTexture(PortraitTex);
		}
	}

	TArray<FCharacterSkillStaticData> Skills = GDS->GetSkillsByCharacter(CharID);
	PopulateSkillIcons(Skills);
}

void ULobbyCharInfoPanel::PopulateSkillIcons(const TArray<FCharacterSkillStaticData>& Skills)
{
	if (!SkillIconContainer)
	{
		return;
	}

	SkillIconContainer->ClearChildren();

	for (const FCharacterSkillStaticData& Skill : Skills)
	{
		if (Skill.SkillIcon.IsNull())
		{
			continue;
		}

		UTexture2D* IconTex = Skill.SkillIcon.LoadSynchronous();
		if (!IconTex)
		{
			continue;
		}

		UImage* IconImage = NewObject<UImage>(this);

		FSlateBrush Brush;
		Brush.SetResourceObject(IconTex);
		Brush.ImageSize = FVector2D(SkillIconSize, SkillIconSize);
		IconImage->SetBrush(Brush);

		UHorizontalBoxSlot* HSlot = SkillIconContainer->AddChildToHorizontalBox(IconImage);
		if (HSlot)
		{
			HSlot->SetPadding(FMargin(SkillIconPadding, 0.f, SkillIconPadding, 0.f));
		}
	}
}

void ULobbyCharInfoPanel::OnConfirmButtonClicked()
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		KHS_WARN("PlayerController 없음");
		return;
	}

	ARSOutGamePlayerController* OutGamePC = Cast<ARSOutGamePlayerController>(PC);
	if (OutGamePC)
	{
		OutGamePC->OnConfirmClicked();
	}
}

void ULobbyCharInfoPanel::OnBackButtonClicked()
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		KHS_WARN("PlayerController 없음");
		return;
	}

	ARSOutGamePlayerController* OutGamePC = Cast<ARSOutGamePlayerController>(PC);
	if (OutGamePC)
	{
		OutGamePC->OnBackClicked();
	}
}
