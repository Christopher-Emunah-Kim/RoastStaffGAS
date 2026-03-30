// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/WeaponReplaceWidget.h"
#include "Subsystems/EquipmentSubsystem.h"
#include "Subsystems/GameDataSubsystem.h"
#include "Subsystems/UIManagerSubsystem.h"
#include "Data/RuntimeDataStructs.h"
#include "System/LoggingSystem.h"
#include "RoastStaffGAS.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

UWeaponReplaceWidget::UWeaponReplaceWidget()
{
	UILayer  = EUILayer::POPUP;
	bIsModal = true;
}

void UWeaponReplaceWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	Btn_Select1->OnClicked.AddDynamic(this, &UWeaponReplaceWidget::OnBtn_Select1Clicked);
	Btn_Select2->OnClicked.AddDynamic(this, &UWeaponReplaceWidget::OnBtn_Select2Clicked);
	Btn_Select3->OnClicked.AddDynamic(this, &UWeaponReplaceWidget::OnBtn_Select3Clicked);
	Btn_Confirm1->OnClicked.AddDynamic(this, &UWeaponReplaceWidget::OnBtn_Confirm1Clicked);
	Btn_Confirm2->OnClicked.AddDynamic(this, &UWeaponReplaceWidget::OnBtn_Confirm2Clicked);
	Btn_Confirm3->OnClicked.AddDynamic(this, &UWeaponReplaceWidget::OnBtn_Confirm3Clicked);
}

void UWeaponReplaceWidget::OpenUI()
{
	SelectedSlotIndex = -1;
	Super::OpenUI();
}

void UWeaponReplaceWidget::InitWidget(FName InPendingWeaponID)
{
	PendingWeaponID = InPendingWeaponID;
	RefreshSlotUI();
}

void UWeaponReplaceWidget::RefreshSlotUI()
{
	GET_GI_SUBSYSTEM_FROM(UEquipmentSubsystem, EQS, GetWorld()->GetGameInstance());
	GET_GI_SUBSYSTEM_FROM(UGameDataSubsystem,  GDS, GetWorld()->GetGameInstance());

	struct FCardWidgets
	{
		UButton*    Select;
		UButton*    Confirm;
		UTextBlock* Name;
		UTextBlock* Level;
		UImage*     Icon;
		UImage*     Highlight;
	};

	const FCardWidgets Cards[] = {
		{ Btn_Select1, Btn_Confirm1, Txt_WeaponName1, Txt_WeaponLevel1, Img_WeaponIcon1, Img_Highlight1 },
		{ Btn_Select2, Btn_Confirm2, Txt_WeaponName2, Txt_WeaponLevel2, Img_WeaponIcon2, Img_Highlight2 },
		{ Btn_Select3, Btn_Confirm3, Txt_WeaponName3, Txt_WeaponLevel3, Img_WeaponIcon3, Img_Highlight3 },
	};

	for (int32 i = 0; i < 3; ++i)
	{
		Cards[i].Confirm->SetIsEnabled(false);
		Cards[i].Highlight->SetVisibility(ESlateVisibility::Hidden);

		const FWeaponSlotInstanceData* SlotInstanceData = EQS->GetSlotData(i);
		if (!SlotInstanceData || SlotInstanceData->IsEmpty())
		{
			Cards[i].Select->SetIsEnabled(false);
			Cards[i].Name->SetText(FText::FromString(TEXT("(빈 슬롯)")));
			Cards[i].Level->SetText(FText::GetEmpty());
			continue;
		}

		Cards[i].Select->SetIsEnabled(true);
		Cards[i].Name->SetText(FText::FromName(SlotInstanceData->SlotEquipData.WeaponName));

		// WeaponLevel 조회 → 레벨 텍스트 표시
		FWeaponStaticData WeaponData;
		if (GDS->GetWeaponData(SlotInstanceData->SlotEquipData.WeaponID, WeaponData))
		{
			Cards[i].Level->SetText(FText::FromString(FString::Printf(TEXT("Lv.%d"), WeaponData.WeaponLevel)));
		}

		// 스킬 아이콘
		if (UTexture2D* Icon = SlotInstanceData->SlotEquipData.SkillIcon.LoadSynchronous())
		{
			Cards[i].Icon->SetBrushFromTexture(Icon);
		}
	}
}

void UWeaponReplaceWidget::OnSlotSelected(int32 SlotIndex)
{
	SelectedSlotIndex = SlotIndex;

	UImage*  Highlights[] = { Img_Highlight1, Img_Highlight2, Img_Highlight3 };
	UButton* Confirms[]   = { Btn_Confirm1,   Btn_Confirm2,   Btn_Confirm3   };

	for (int32 i = 0; i < 3; ++i)
	{
		const bool bSelected = (i == SlotIndex);
		Highlights[i]->SetVisibility(bSelected ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
		Confirms[i]->SetIsEnabled(bSelected);
	}

	KHS_INFO(TEXT("[WeaponReplace] 슬롯 선택 — Index: %d"), SlotIndex);
}

void UWeaponReplaceWidget::OnConfirmClicked(int32 SlotIndex)
{
	if (PendingWeaponID.IsNone())
	{
		KHS_WARN(TEXT("[WeaponReplace] PendingWeaponID 없음. 확인 무시."));
		return;
	}

	GET_GI_SUBSYSTEM_FROM(UEquipmentSubsystem, EQS, GetWorld()->GetGameInstance());
	EQS->UpgradeWeapon(SlotIndex, PendingWeaponID);

	KHS_INFO(TEXT("[WeaponReplace] 교체 확인 — Slot:%d ← %s"), SlotIndex, *PendingWeaponID.ToString());

	GET_GI_SUBSYSTEM_FROM(UUIManagerSubsystem, UMS, GetWorld()->GetGameInstance());
	UMS->CloseUI(this);

	OnReplaceCompletedDel.Broadcast();
}

// ── 버튼 핸들러 ────────────────────────────────────────────────────────────
void UWeaponReplaceWidget::OnBtn_Select1Clicked()  { OnSlotSelected(0); }
void UWeaponReplaceWidget::OnBtn_Select2Clicked()  { OnSlotSelected(1); }
void UWeaponReplaceWidget::OnBtn_Select3Clicked()  { OnSlotSelected(2); }
void UWeaponReplaceWidget::OnBtn_Confirm1Clicked() { OnConfirmClicked(0); }
void UWeaponReplaceWidget::OnBtn_Confirm2Clicked() { OnConfirmClicked(1); }
void UWeaponReplaceWidget::OnBtn_Confirm3Clicked() { OnConfirmClicked(2); }
