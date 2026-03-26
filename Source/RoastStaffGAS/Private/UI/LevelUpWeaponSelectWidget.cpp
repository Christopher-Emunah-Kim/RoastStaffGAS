// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/LevelUpWeaponSelectWidget.h"
#include "Subsystems/EquipmentSubsystem.h"
#include "Subsystems/UIManagerSubsystem.h"
#include "System/LoggingSystem.h"
#include "RoastStaffGAS.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

namespace
{
	FText CardStateToLevelText(EWeaponCardState State)
	{
		switch (State)
		{
			case EWeaponCardState::New:       return FText::FromString(TEXT("NEW"));
			case EWeaponCardState::Lv1ToLv2:  return FText::FromString(TEXT("Lv.1→2"));
			case EWeaponCardState::Lv2ToLv3:  return FText::FromString(TEXT("Lv.2→3"));
			case EWeaponCardState::Lv3Max:    return FText::FromString(TEXT("MAX"));
			default:                          return FText::GetEmpty();
		}
	}
}

ULevelUpWeaponSelectWidget::ULevelUpWeaponSelectWidget()
{
	UILayer  = EUILayer::POPUP;
	bIsModal = true;
}

void ULevelUpWeaponSelectWidget::NativeConstruct()
{
	Super::NativeConstruct();

	Btn_Select1->OnClicked.AddDynamic(this, &ULevelUpWeaponSelectWidget::OnBtn_Select1Clicked);
	Btn_Select2->OnClicked.AddDynamic(this, &ULevelUpWeaponSelectWidget::OnBtn_Select2Clicked);
	Btn_Select3->OnClicked.AddDynamic(this, &ULevelUpWeaponSelectWidget::OnBtn_Select3Clicked);
	Btn_Confirm1->OnClicked.AddDynamic(this, &ULevelUpWeaponSelectWidget::OnBtn_Confirm1Clicked);
	Btn_Confirm2->OnClicked.AddDynamic(this, &ULevelUpWeaponSelectWidget::OnBtn_Confirm2Clicked);
	Btn_Confirm3->OnClicked.AddDynamic(this, &ULevelUpWeaponSelectWidget::OnBtn_Confirm3Clicked);
	Btn_Close->OnClicked.AddDynamic(this, &ULevelUpWeaponSelectWidget::OnBtn_CloseClicked);
}

void ULevelUpWeaponSelectWidget::OpenUI()
{
	SelectedCardIndex = -1;
	Candidates.Empty();
	Super::OpenUI();
}

void ULevelUpWeaponSelectWidget::SetCandidates(const TArray<FWeaponCardDisplayData>& InCandidates)
{
	if (!ensureMsgf(!InCandidates.IsEmpty(), TEXT("SetCandidates: 후보 배열이 비어있음")))
	{
		return;
	}
	Candidates = InCandidates;
	RefreshCandidateUI();
}

void ULevelUpWeaponSelectWidget::RefreshCandidateUI()
{
	struct FCardWidgets
	{
		UButton*    Select;
		UButton*    Confirm;
		UTextBlock* Name;
		UTextBlock* Desc;
		UTextBlock* Level;
		UImage*     Icon;
		UImage*     Highlight;
	};

	const FCardWidgets Cards[] = {
		{ Btn_Select1, Btn_Confirm1, Txt_WeaponName1, Txt_Desc1, Txt_WeaponLevel1, Img_WeaponIcon1, Img_Highlight1 },
		{ Btn_Select2, Btn_Confirm2, Txt_WeaponName2, Txt_Desc2, Txt_WeaponLevel2, Img_WeaponIcon2, Img_Highlight2 },
		{ Btn_Select3, Btn_Confirm3, Txt_WeaponName3, Txt_Desc3, Txt_WeaponLevel3, Img_WeaponIcon3, Img_Highlight3 },
	};

	// Txt_Explain: 카드 미선택 상태에서 표시
	Txt_Explain->SetVisibility(ESlateVisibility::HitTestInvisible);

	for (int32 i = 0; i < 3; ++i)
	{
		const bool bValid = Candidates.IsValidIndex(i);

		// Select 버튼: 유효 후보가 있을 때만 활성
		Cards[i].Select->SetIsEnabled(bValid);
		// Confirm 버튼: 초기에는 항상 비활성 (카드 선택 후 활성화)
		Cards[i].Confirm->SetIsEnabled(false);
		// 하이라이트: 초기에는 항상 Hidden
		Cards[i].Highlight->SetVisibility(ESlateVisibility::Hidden);

		if (bValid)
		{
			Cards[i].Name->SetText(FText::FromName(Candidates[i].WeaponName));
			Cards[i].Desc->SetText(FText::FromName(Candidates[i].Description));
			Cards[i].Level->SetText(CardStateToLevelText(Candidates[i].CardState));

			if (UTexture2D* Icon = Candidates[i].WeaponIcon.LoadSynchronous())
			{
				Cards[i].Icon->SetBrushFromTexture(Icon);
			}
		}
		else
		{
			Cards[i].Name->SetText(FText::GetEmpty());
			Cards[i].Desc->SetText(FText::GetEmpty());
			Cards[i].Level->SetText(FText::GetEmpty());
		}
	}
}

void ULevelUpWeaponSelectWidget::OnCardSelected(int32 CardIndex)
{
	if (!Candidates.IsValidIndex(CardIndex))
	{
		KHS_WARN(TEXT("OnCardSelected: 유효하지 않은 CardIndex %d"), CardIndex);
		return;
	}

	SelectedCardIndex = CardIndex;

	// Txt_Explain: 카드 선택 시점부터 숨김
	Txt_Explain->SetVisibility(ESlateVisibility::Collapsed);

	// 카드별 하이라이트 & Confirm 버튼 활성/비활성 갱신
	UImage*  Highlights[] = { Img_Highlight1, Img_Highlight2, Img_Highlight3 };
	UButton* Confirms[]   = { Btn_Confirm1,   Btn_Confirm2,   Btn_Confirm3   };

	for (int32 i = 0; i < 3; ++i)
	{
		const bool bSelected = (i == CardIndex);
		Highlights[i]->SetVisibility(bSelected ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
		Confirms[i]->SetIsEnabled(bSelected);
	}

	KHS_INFO(TEXT("카드 선택 — Index: %d, WeaponID: %s"), CardIndex, *Candidates[CardIndex].WeaponID.ToString());
}

void ULevelUpWeaponSelectWidget::EquipAndClose(int32 CardIndex)
{
	if (!Candidates.IsValidIndex(CardIndex))
	{
		KHS_WARN(TEXT("EquipAndClose: 유효하지 않은 CardIndex %d"), CardIndex);
		return;
	}

	GET_GI_SUBSYSTEM_FROM(UEquipmentSubsystem, EquipSys, GetWorld()->GetGameInstance());
	EquipSys->EquipWeapon(Candidates[CardIndex].WeaponID);
	KHS_INFO(TEXT("무기 장착 — WeaponID: %s"), *Candidates[CardIndex].WeaponID.ToString());

	GET_GI_SUBSYSTEM_FROM(UUIManagerSubsystem, UMS, GetWorld()->GetGameInstance());
	UMS->CloseUI(this);
	OnWeaponSelectCompletedDel.Broadcast();
}

void ULevelUpWeaponSelectWidget::CloseWithoutEquip()
{
	KHS_INFO(TEXT("레벨업 UI 스킵 — 무기 미선택"));
	GET_GI_SUBSYSTEM_FROM(UUIManagerSubsystem, UMS, GetWorld()->GetGameInstance());
	UMS->CloseUI(this);
	OnWeaponSelectCompletedDel.Broadcast();
}

// ── 버튼 핸들러 ──────────────────────────────────────────────────────────────
void ULevelUpWeaponSelectWidget::OnBtn_Select1Clicked()  { OnCardSelected(0); }
void ULevelUpWeaponSelectWidget::OnBtn_Select2Clicked()  { OnCardSelected(1); }
void ULevelUpWeaponSelectWidget::OnBtn_Select3Clicked()  { OnCardSelected(2); }
void ULevelUpWeaponSelectWidget::OnBtn_Confirm1Clicked() { EquipAndClose(0); }
void ULevelUpWeaponSelectWidget::OnBtn_Confirm2Clicked() { EquipAndClose(1); }
void ULevelUpWeaponSelectWidget::OnBtn_Confirm3Clicked() { EquipAndClose(2); }
void ULevelUpWeaponSelectWidget::OnBtn_CloseClicked()    { CloseWithoutEquip(); }
