// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/RSBaseWidget.h"
#include "UI/OutGame/RSCharacterEntryWidget.h"
#include "RSCharacterGridPopupWidget.generated.h"

class UWrapBox;
class UButton;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterFocusRequested, FName, CharID);

/**
 * URSCharacterGridPopupWidget
 *
 * 전체 캐릭터 목록을 그리드로 표시하는 POPUP 위젯.
 * 해금 → 등급(SSR>SR>R>N) → 레벨(stub) 순으로 정렬.
 * 엔트리 선택 시 OnCharacterFocusRequestedDel 브로드캐스트 + 팝업 자동 닫기.
 */
UCLASS()
class ROASTSTAFFGAS_API URSCharacterGridPopupWidget : public URSBaseWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;

public:
	/** GDS + SGS에서 전체 캐릭터를 조회·정렬해 GridContainer를 채운다 */
	UFUNCTION(BlueprintCallable, Category = "RS|CharacterGridPopup")
	void PopulateGrid();
	
protected:
	/** Btn_Close 클릭 → CloseTopPopupUI */
	UFUNCTION() void OnCloseClicked();
	/** Entry 클릭 → 델리게이트 브로드캐스트 + 팝업 닫기 */
	UFUNCTION() void OnEntryClicked(FName CharID);

public:
	/** 엔트리 선택 시 캐러셀 포커스 요청 — RSCharacterSelectWidget::OnCharacterFocusFromGrid 바인딩 대상 */
	UPROPERTY(BlueprintAssignable, Category = "RS|CharacterGridPopup")
	FOnCharacterFocusRequested OnCharacterFocusRequestedDel;

protected:
	/** 팝업 닫기 버튼 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Close;
	/** 캐릭터 카드 그리드 컨테이너 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWrapBox> GridContainer;

	/** 생성할 Entry 위젯 클래스 — WBP EditDefaultsOnly로 할당 */
	UPROPERTY(EditDefaultsOnly, Category = "RS|CharacterGridPopup")
	TSubclassOf<URSCharacterEntryWidget> EntryWidgetClass;
};
