// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/LevelUpWeaponSelectWidget.h"
#include "Subsystems/EquipmentSubsystem.h"
#include "Subsystems/LevelUpSubsystem.h"
#include "Subsystems/UIManagerSubsystem.h"
#include "System/LoggingSystem.h"
#include "RoastStaffGAS.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

namespace
{
	FText CardTypeToLabel(ELevelUpCardType CardType)
	{
		switch (CardType)
		{
			case ELevelUpCardType::StatUpgrade:   return FText::FromString(TEXT("스탯 업그레이드"));
			case ELevelUpCardType::PassiveAdd:    return FText::FromString(TEXT("패시브"));
			case ELevelUpCardType::WeaponUpgrade: return FText::FromString(TEXT("무기 강화"));
			case ELevelUpCardType::WeaponNew:     return FText::FromString(TEXT("신규 무기"));
			
			default:                              return FText::GetEmpty();
		}
	}
}

ULevelUpWeaponSelectWidget::ULevelUpWeaponSelectWidget()
{
	UILayer  = EUILayer::POPUP;
	bIsModal = true;
}

void ULevelUpWeaponSelectWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	Btn_Select1->OnClicked.AddDynamic(this, &ULevelUpWeaponSelectWidget::OnBtn_Select1Clicked);
	Btn_Select2->OnClicked.AddDynamic(this, &ULevelUpWeaponSelectWidget::OnBtn_Select2Clicked);
	Btn_Select3->OnClicked.AddDynamic(this, &ULevelUpWeaponSelectWidget::OnBtn_Select3Clicked);
	Btn_Select4->OnClicked.AddDynamic(this, &ULevelUpWeaponSelectWidget::OnBtn_Select4Clicked);
	Btn_Confirm1->OnClicked.AddDynamic(this, &ULevelUpWeaponSelectWidget::OnBtn_Confirm1Clicked);
	Btn_Confirm2->OnClicked.AddDynamic(this, &ULevelUpWeaponSelectWidget::OnBtn_Confirm2Clicked);
	Btn_Confirm3->OnClicked.AddDynamic(this, &ULevelUpWeaponSelectWidget::OnBtn_Confirm3Clicked);
	Btn_Confirm4->OnClicked.AddDynamic(this, &ULevelUpWeaponSelectWidget::OnBtn_Confirm4Clicked);
	Btn_Close->OnClicked.AddDynamic(this, &ULevelUpWeaponSelectWidget::OnBtn_CloseClicked);
}

void ULevelUpWeaponSelectWidget::OpenUI()
{
	SelectedCardIndex = -1;
	Cards.Empty();
	Super::OpenUI();
}

void ULevelUpWeaponSelectWidget::SetCards(const TArray<FLevelUpCardDisplayData>& InCards)
{
	if (!ensureMsgf(!InCards.IsEmpty(), TEXT("SetCards: 카드 배열이 비어있음")))
	{
		return;
	}
	Cards = InCards;
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

	const FCardWidgets Widgets[] = {
		{ Btn_Select1, Btn_Confirm1, Txt_WeaponName1, Txt_Desc1, Txt_WeaponLevel1, Img_CardIcon1, Img_Highlight1 },
		{ Btn_Select2, Btn_Confirm2, Txt_WeaponName2, Txt_Desc2, Txt_WeaponLevel2, Img_CardIcon2, Img_Highlight2 },
		{ Btn_Select3, Btn_Confirm3, Txt_WeaponName3, Txt_Desc3, Txt_WeaponLevel3, Img_CardIcon3, Img_Highlight3 },
		{ Btn_Select4, Btn_Confirm4, Txt_WeaponName4, Txt_Desc4, Txt_WeaponLevel4, Img_CardIcon4, Img_Highlight4 },
	};

	// Txt_Explain: 카드 미선택 상태에서 표시
	Txt_Explain->SetVisibility(ESlateVisibility::HitTestInvisible);

	for (int32 i = 0; i < 4; ++i)
	{
		const bool bValid = Cards.IsValidIndex(i);

		// Select 버튼: 유효 카드가 있을 때만 활성
		Widgets[i].Select->SetIsEnabled(bValid);
		// Confirm 버튼: 초기에는 항상 비활성 (카드 선택 후 활성화)
		Widgets[i].Confirm->SetIsEnabled(false);
		// 하이라이트: 초기에는 항상 Hidden
		Widgets[i].Highlight->SetVisibility(ESlateVisibility::Hidden);

		if (bValid)
		{
			Widgets[i].Name->SetText(Cards[i].DisplayName);
			Widgets[i].Desc->SetText(Cards[i].Description);
			Widgets[i].Level->SetText(CardTypeToLabel(Cards[i].CardType));

			if (UTexture2D* Icon = Cards[i].Icon.LoadSynchronous())
			{
				Widgets[i].Icon->SetBrushFromTexture(Icon);
			}
		}
		else
		{
			Widgets[i].Name->SetText(FText::GetEmpty());
			Widgets[i].Desc->SetText(FText::GetEmpty());
			Widgets[i].Level->SetText(FText::GetEmpty());
		}
	}
}

void ULevelUpWeaponSelectWidget::OnCardSelected(int32 CardIndex)
{
	if (!Cards.IsValidIndex(CardIndex))
	{
		KHS_WARN(TEXT("OnCardSelected: 유효하지 않은 CardIndex %d"), CardIndex);
		return;
	}

	SelectedCardIndex = CardIndex;

	// Txt_Explain: 카드 선택 시점부터 숨김
	Txt_Explain->SetVisibility(ESlateVisibility::Collapsed);

	// 카드별 하이라이트 & Confirm 버튼 활성/비활성 갱신
	UImage*  Highlights[] = { Img_Highlight1, Img_Highlight2, Img_Highlight3, Img_Highlight4 };
	UButton* Confirms[]   = { Btn_Confirm1,   Btn_Confirm2,   Btn_Confirm3,   Btn_Confirm4   };

	for (int32 i = 0; i < 4; ++i)
	{
		const bool bSelected = (i == CardIndex);
		Highlights[i]->SetVisibility(bSelected ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
		Confirms[i]->SetIsEnabled(bSelected);
	}

	KHS_INFO(TEXT("카드 선택 — Index: %d, CardID: %s"), CardIndex, *Cards[CardIndex].CardID.ToString());
}

void ULevelUpWeaponSelectWidget::EquipAndClose(int32 CardIndex)
{
	if (!Cards.IsValidIndex(CardIndex))
	{
		KHS_WARN(TEXT("EquipAndClose: 유효하지 않은 CardIndex %d"), CardIndex);
		return;
	}

	GET_GI_SUBSYSTEM_FROM(ULevelUpSubsystem, LevelUpSys, GetWorld()->GetGameInstance())
	LevelUpSys->OnCardSelected(Cards[CardIndex].CardID);

	GET_GI_SUBSYSTEM_FROM(UUIManagerSubsystem, UMS, GetWorld()->GetGameInstance())
	UMS->CloseUI(this);

	// 무기 카드 + 슬롯 가득 → OnSlotFull 발행된 경우 교체 UI가 게임 재개를 담당
	const ELevelUpCardType CardType = Cards[CardIndex].CardType;
	if (CardType == ELevelUpCardType::WeaponNew || CardType == ELevelUpCardType::WeaponUpgrade)
	{
		GET_GI_SUBSYSTEM_FROM(UEquipmentSubsystem, EquipSys, GetWorld()->GetGameInstance())
		if (EquipSys->PendingWeaponID != NAME_None)
		{
			return;
		}
	}

	OnWeaponSelectCompletedDel.Broadcast();
}

void ULevelUpWeaponSelectWidget::CloseWithoutEquip()
{
	KHS_INFO(TEXT("레벨업 UI 스킵 — 카드 미선택"));
	GET_GI_SUBSYSTEM_FROM(UUIManagerSubsystem, UMS, GetWorld()->GetGameInstance())
	UMS->CloseUI(this);
	OnWeaponSelectCompletedDel.Broadcast();
}

// ── 버튼 핸들러 ──────────────────────────────────────────────────────────────
void ULevelUpWeaponSelectWidget::OnBtn_Select1Clicked()  { OnCardSelected(0); }
void ULevelUpWeaponSelectWidget::OnBtn_Select2Clicked()  { OnCardSelected(1); }
void ULevelUpWeaponSelectWidget::OnBtn_Select3Clicked()  { OnCardSelected(2); }
void ULevelUpWeaponSelectWidget::OnBtn_Select4Clicked()  { OnCardSelected(3); }
void ULevelUpWeaponSelectWidget::OnBtn_Confirm1Clicked() { EquipAndClose(0); }
void ULevelUpWeaponSelectWidget::OnBtn_Confirm2Clicked() { EquipAndClose(1); }
void ULevelUpWeaponSelectWidget::OnBtn_Confirm3Clicked() { EquipAndClose(2); }
void ULevelUpWeaponSelectWidget::OnBtn_Confirm4Clicked() { EquipAndClose(3); }
void ULevelUpWeaponSelectWidget::OnBtn_CloseClicked()    { CloseWithoutEquip(); }
