// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/OutGame/RSCharacterEntryWidget.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Overlay.h"

// LOCKED 상태 초상화에 적용할 틴트 (어두운 실루엣)
static const FLinearColor GPortraitLockedTint   = FLinearColor(0.08f, 0.08f, 0.08f, 1.0f);
static const FLinearColor GPortraitUnlockedTint = FLinearColor::White;

// ECharacterGrade → 표시 문자열 변환
static FText GetGradeText(ECharacterGrade Grade)
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

void URSCharacterEntryWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// 버튼 클릭 바인딩 — NativeOnInitialized에 배치(풀링 재사용 시 중복 방지)
	if (Btn_Card)
	{
		Btn_Card->OnClicked.AddDynamic(this, &URSCharacterEntryWidget::OnCardClicked);
	}
}

void URSCharacterEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// SetupEntry가 구성 이전에 호출된 경우 여기서 위젯에 반영
	UpdateDisplay();
}

void URSCharacterEntryWidget::SetupEntry(const FCharacterStaticData& InData, bool bInUnlocked)
{
	CharData  = InData;
	bUnlocked = bInUnlocked;

	// 위젯이 이미 구성된 상태(캐러셀 재구성 등)이면 즉시 반영
	if (IsConstructed())
	{
		UpdateDisplay();
	}
}

void URSCharacterEntryWidget::UpdateDisplay()
{
	// 초상화 
	if (Img_Portrait)
	{
		// TODO(PLAN_CharAsset): 비동기 로드로 교체 — 현재는 UI 오픈 시점에만 호출되므로 동기 로드 허용
		UTexture2D* Tex = CharData.Portrait.LoadSynchronous();
		if (Tex)
		{
			Img_Portrait->SetBrushFromTexture(Tex);
		}

		// LOCKED → 어두운 틴트로 실루엣 효과
		Img_Portrait->SetColorAndOpacity(bUnlocked ? GPortraitUnlockedTint : GPortraitLockedTint);
	}

	// 잠금 오버레이 (자물쇠 아이콘/딤)
	if (Overlay_Locked)
	{
		Overlay_Locked->SetVisibility(bUnlocked ? ESlateVisibility::Collapsed: ESlateVisibility::SelfHitTestInvisible);
	}

	// 등급 텍스트
	if (Txt_Grade)
	{
		Txt_Grade->SetText(GetGradeText(CharData.Grade));
	}

	// 캐릭터 이름 (LOCKED 시 Hidden)
	if (Txt_Name)
	{
		if (bUnlocked)
		{
			Txt_Name->SetText(CharData.DisplayName);
			Txt_Name->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
		else
		{
			Txt_Name->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	// 버튼 활성화 (LOCKED 카드는 클릭 불가) 
	if (Btn_Card)
	{
		Btn_Card->SetIsEnabled(bUnlocked);
	}
}

void URSCharacterEntryWidget::OnCardClicked()
{
	// Btn_Card가 LOCKED 시 비활성화되어 있으므로 여기 도달 = 항상 해금 상태
	OnEntryClickedDel.Broadcast(CharData.CharacterID);
}
