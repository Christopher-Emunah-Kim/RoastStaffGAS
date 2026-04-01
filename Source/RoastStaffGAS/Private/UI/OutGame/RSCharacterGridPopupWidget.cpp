// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/OutGame/RSCharacterGridPopupWidget.h"
#include "Subsystems/UIManagerSubsystem.h"
#include "Subsystems/GameDataSubsystem.h"
#include "Subsystems/SaveGameSubsystem.h"
#include "Data/DataTableStructs.h"
#include "System/LoggingSystem.h"
#include "RoastStaffGAS.h"
#include "Components/WrapBox.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void URSCharacterGridPopupWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// 버튼 바인딩 — NativeOnInitialized에 배치(풀링 재사용 시 중복 방지)
	if (Btn_Close)
	{
		Btn_Close->OnClicked.AddDynamic(this, &URSCharacterGridPopupWidget::OnCloseClicked);
	}
}

void URSCharacterGridPopupWidget::NativeConstruct()
{
	Super::NativeConstruct();

	PopulateGrid();
}

void URSCharacterGridPopupWidget::PopulateGrid()
{
	if (!GridContainer)
	{
		KHS_WARN(TEXT("GridContainer가 null입니다."));
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
		return;
	}

	// 정렬: 해금 여부(해금 우선) → Grade 오름차순(SSR=0 앞) → Level stub(동순위 유지)
	AllChars.Sort([&SGS](const FCharacterStaticData& A, const FCharacterStaticData& B)
	{
		const bool bAUnlocked = (A.UnlockType == ECharacterUnlockType::DEFAULT)
		                      || SGS->IsCharacterUnlocked(A.CharacterID);
		const bool bBUnlocked = (B.UnlockType == ECharacterUnlockType::DEFAULT)
		                      || SGS->IsCharacterUnlocked(B.CharacterID);

		if (bAUnlocked != bBUnlocked)
		{
			return bAUnlocked > bBUnlocked;
		}

		if (A.Grade != B.Grade)
		{
			return static_cast<uint8>(A.Grade) < static_cast<uint8>(B.Grade);
		}

		// TODO(PLAN_RDS): SGS 캐릭터 레벨 조회 후 정렬 추가
		return false;
	});

	GridContainer->ClearChildren();

	for (const FCharacterStaticData& CharData : AllChars)
	{
		bool bUnlocked = (CharData.UnlockType == ECharacterUnlockType::DEFAULT)
		               || SGS->IsCharacterUnlocked(CharData.CharacterID);

		TObjectPtr<URSCharacterEntryWidget> Entry = CreateWidget<URSCharacterEntryWidget>(this, EntryWidgetClass);
		if (!Entry)
		{
			continue;
		}

		Entry->SetupEntry(CharData, bUnlocked);

		// 중복 바인딩 방지 후 클릭 이벤트 구독
		Entry->OnEntryClickedDel.RemoveDynamic(this, &URSCharacterGridPopupWidget::OnEntryClicked);
		Entry->OnEntryClickedDel.AddDynamic(this, &URSCharacterGridPopupWidget::OnEntryClicked);

		GridContainer->AddChild(Entry);
	}
}

void URSCharacterGridPopupWidget::OnCloseClicked()
{
	GET_GI_SUBSYSTEM_FROM(UUIManagerSubsystem, UMS, GetWorld()->GetGameInstance());

	UMS->CloseTopPopupUI();
}

void URSCharacterGridPopupWidget::OnEntryClicked(FName CharID)
{
	// 구독자(RSCharacterSelectWidget)에게 캐러셀 포커스 이동 요청
	OnCharacterFocusRequestedDel.Broadcast(CharID);

	// 선택 완료 → 팝업 닫기
	GET_GI_SUBSYSTEM_FROM(UUIManagerSubsystem, UMS, GetWorld()->GetGameInstance());

	UMS->CloseTopPopupUI();
}
