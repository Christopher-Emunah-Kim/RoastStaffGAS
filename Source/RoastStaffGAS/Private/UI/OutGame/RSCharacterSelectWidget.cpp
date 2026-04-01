// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/OutGame/RSCharacterSelectWidget.h"
#include "UI/OutGame/RSCharacterGridPopupWidget.h"
#include "Subsystems/UIManagerSubsystem.h"
#include "Subsystems/GameDataSubsystem.h"
#include "Subsystems/SaveGameSubsystem.h"
#include "Data/EnumUITypes.h"
#include "System/LoggingSystem.h"
#include "RoastStaffGAS.h"
#include "Components/ScrollBox.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

// ECharacterGrade → 표시 문자열 (EntryWidget과 동일 변환 — 공용 헬퍼로 분리 시 TODO)
static FText GetGradeTextForSelect(ECharacterGrade Grade)
{
	switch (Grade)
	{
	case ECharacterGrade::SSR: return FText::FromString(TEXT("SSR"));
	case ECharacterGrade::SR:  return FText::FromString(TEXT("SR"));
	case ECharacterGrade::R:   return FText::FromString(TEXT("R"));
	case ECharacterGrade::N:   return FText::FromString(TEXT("N"));
	default:                   return FText::FromString(TEXT("?"));
	}
}

// -----------------------------------------------------------------------------

void URSCharacterSelectWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// 버튼 바인딩 — NativeOnInitialized에 배치(풀링 재사용 시 중복 방지)
	if (Btn_Back)
	{
		Btn_Back->OnClicked.AddDynamic(this, &URSCharacterSelectWidget::OnBackClicked);
	}

	if (Btn_GridPopup)
	{
		Btn_GridPopup->OnClicked.AddDynamic(this, &URSCharacterSelectWidget::OnGridPopupClicked);
	}

	if (Btn_StageSelect)
	{
		Btn_StageSelect->OnClicked.AddDynamic(this, &URSCharacterSelectWidget::OnStageSelectClicked);
	}
}

void URSCharacterSelectWidget::NativeConstruct()
{
	Super::NativeConstruct();

	PopulateCarousel();
}

void URSCharacterSelectWidget::PopulateCarousel()
{
	if (!CarouselContainer)
	{
		KHS_WARN(TEXT("CarouselContainer가 null입니다."));
		return;
	}

	if (!EntryWidgetClass)
	{
		KHS_WARN(TEXT("EntryWidgetClass가 설정되지 않았습니다."));
		return;
	}

	GET_GI_SUBSYSTEM_FROM(UGameDataSubsystem, GDS, GetWorld()->GetGameInstance());
	GET_GI_SUBSYSTEM_FROM(USaveGameSubsystem, SGS, GetWorld()->GetGameInstance());

	TArray<FCharacterStaticData> AllChars;
	if (!GDS->GetAllCharacterStaticData(AllChars))
	{
		KHS_WARN(TEXT("캐릭터 데이터 조회 실패. 빈 목록으로 표시."));
		ClearInfoPanel();
		return;
	}

	CarouselContainer->ClearChildren();
	EntryWidgets.Empty();
	ClearInfoPanel();

	for (const FCharacterStaticData& CharData : AllChars)
	{
		// DEFAULT 타입은 항상 해금 — SGS는 STAGE_CLEAR/CURRENCY로 해금된 캐릭터만 추적
		bool bUnlocked = (CharData.UnlockType == ECharacterUnlockType::DEFAULT)
		               || SGS->IsCharacterUnlocked(CharData.CharacterID);

		TObjectPtr<URSCharacterEntryWidget> Entry = CreateWidget<URSCharacterEntryWidget>(this, EntryWidgetClass);
		if (!Entry)
		{
			continue;
		}

		Entry->SetupEntry(CharData, bUnlocked);

		// 중복 바인딩 방지 후 클릭 이벤트 구독
		Entry->OnEntryClickedDel.RemoveDynamic(this, &URSCharacterSelectWidget::OnCharacterEntryClicked);
		Entry->OnEntryClickedDel.AddDynamic(this, &URSCharacterSelectWidget::OnCharacterEntryClicked);

		CarouselContainer->AddChild(Entry);
		EntryWidgets.Add(Entry);
	}

	// 초기 포커스: 마지막 선택 캐릭터 → 없으면 첫 번째 해금 캐릭터 → 없으면 첫 번째 카드
	FName InitialFocus = SGS->GetLastSelectedCharacter();
	if (InitialFocus.IsNone())
	{
		// 첫 번째 해금 캐릭터 탐색
		for (const URSCharacterEntryWidget* Entry : EntryWidgets)
		{
			if (Entry && Entry->IsUnlocked())
			{
				InitialFocus = Entry->GetCharacterID();
				break;
			}
		}
	}

	if (InitialFocus.IsNone() && EntryWidgets.Num() > 0 && EntryWidgets[0])
	{
		// 해금 캐릭터 없음 — 첫 번째 카드 포커스
		InitialFocus = EntryWidgets[0]->GetCharacterID();
	}

	if (!InitialFocus.IsNone())
	{
		FocusCarouselOn(InitialFocus);
	}
}

void URSCharacterSelectWidget::FocusCarouselOn(FName CharID)
{
	if (CharID.IsNone())
	{
		UE_LOG(LogTemp, Verbose, TEXT("FocusCarouselOn — CharID가 None입니다. 무시."));
		return;
	}

	for (URSCharacterEntryWidget* Entry : EntryWidgets)
	{
		if (!Entry)
		{
			continue;
		}

		if (Entry->GetCharacterID() == CharID)
		{
			// 해당 카드가 화면에 보이도록 스크롤
			CarouselContainer->ScrollWidgetIntoView(Entry, true);
			break;
		}
	}

	// 포커스 이동과 함께 정보 패널 갱신
	UpdateInfoPanel(CharID);
}

// ── 버튼 핸들러 ───────────────────────────────────────────────────────────────

void URSCharacterSelectWidget::OnBackClicked()
{
	GET_GI_SUBSYSTEM_FROM(UUIManagerSubsystem, UMS, GetWorld()->GetGameInstance());

	UMS->BackPage();
}

void URSCharacterSelectWidget::OnGridPopupClicked()
{
	GET_GI_SUBSYSTEM_FROM(UUIManagerSubsystem, UMS, GetWorld()->GetGameInstance());

	URSBaseWidget* RawWidget = UMS->OpenUIByID(EUIID::CHAR_GRID_POPUP);
	URSCharacterGridPopupWidget* GridPopup = Cast<URSCharacterGridPopupWidget>(RawWidget);
	if (!GridPopup)
	{
		KHS_WARN(TEXT("CHAR_GRID_POPUP 위젯을 캐스트할 수 없습니다."));
		return;
	}

	// 중복 바인딩 방지 후 포커스 요청 델리게이트 구독
	GridPopup->OnCharacterFocusRequestedDel.RemoveDynamic(this, &URSCharacterSelectWidget::OnCharacterFocusFromGrid);
	GridPopup->OnCharacterFocusRequestedDel.AddDynamic(this, &URSCharacterSelectWidget::OnCharacterFocusFromGrid);
}

void URSCharacterSelectWidget::OnStageSelectClicked()
{
	// 선택된 캐릭터가 있으면 확정 브로드캐스트 → OGPC가 SGS 저장 + 페이지 전환 처리
	// 없으면 단순 페이지 이동 (OGPC::OnStageSelectClicked 흐름과 동일)
	if (!SelectedCharID.IsNone())
	{
		OnCharacterSelectedDel.Broadcast(SelectedCharID);
		return;
	}

	GET_GI_SUBSYSTEM_FROM(UUIManagerSubsystem, UMS, GetWorld()->GetGameInstance());
	UMS->SwitchPageUI(EUIID::STAGE_SELECT);
}

// ── Entry 이벤트 ──────────────────────────────────────────────────────────────

void URSCharacterSelectWidget::OnCharacterEntryClicked(FName CharID)
{
	// 카드 클릭 → 포커스(정보 패널 갱신) + 선택 저장. 확정은 Btn_StageSelect에서.
	SelectedCharID = CharID;
	UpdateInfoPanel(CharID);
}

void URSCharacterSelectWidget::OnCharacterFocusFromGrid(FName CharID)
{
	// 그리드 팝업에서 선택 → 캐러셀로 포커스 이동
	SelectedCharID = CharID;
	FocusCarouselOn(CharID);
}

// ── 정보 패널 ─────────────────────────────────────────────────────────────────

void URSCharacterSelectWidget::UpdateInfoPanel(FName CharID)
{
	GET_GI_SUBSYSTEM_FROM(UGameDataSubsystem, GDS, GetWorld()->GetGameInstance());

	FCharacterStaticData Data;
	if (!GDS->GetCharacterStaticData(CharID, Data))
	{
		KHS_WARN(TEXT("RSCharacterSelectWidget::UpdateInfoPanel — CharID %s 데이터 조회 실패."), *CharID.ToString());
		ClearInfoPanel();
		return;
	}

	if (Txt_SelectedName)
	{
		Txt_SelectedName->SetText(Data.DisplayName);
	}

	if (Txt_SelectedGrade)
	{
		Txt_SelectedGrade->SetText(GetGradeTextForSelect(Data.Grade));
	}

	if (Txt_StatHP)
	{
		Txt_StatHP->SetText(FText::AsNumber(static_cast<int32>(Data.BaseHP)));
	}

	if (Txt_StatSpeed)
	{
		Txt_StatSpeed->SetText(FText::AsNumber(static_cast<int32>(Data.BaseMoveSpeed)));
	}

	if (Txt_StatAttack)
	{
		Txt_StatAttack->SetText(FText::AsNumber(static_cast<int32>(Data.BaseAttackPower)));
	}
}

void URSCharacterSelectWidget::ClearInfoPanel()
{
	const FText EmptyText = FText::GetEmpty();

	if (Txt_SelectedName)  { Txt_SelectedName->SetText(EmptyText);  }
	if (Txt_SelectedGrade) { Txt_SelectedGrade->SetText(EmptyText); }
	if (Txt_StatHP)        { Txt_StatHP->SetText(EmptyText);        }
	if (Txt_StatSpeed)     { Txt_StatSpeed->SetText(EmptyText);     }
	if (Txt_StatAttack)    { Txt_StatAttack->SetText(EmptyText);    }
}
