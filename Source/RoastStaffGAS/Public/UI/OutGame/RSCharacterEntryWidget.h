// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/RSBaseWidget.h"
#include "Data/DataTableStructs.h"
#include "RSCharacterEntryWidget.generated.h"

class UButton;
class UImage;
class UTextBlock;
class UOverlay;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterEntryClicked, FName, CharID);

/**
 * URSCharacterEntryWidget
 *
 * 캐릭터 선택 캐러셀 및 그리드에서 공용으로 사용하는 캐릭터 카드 위젯.
 */
UCLASS()
class ROASTSTAFFGAS_API URSCharacterEntryWidget : public URSBaseWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;

	
public:
	/** 캐릭터 데이터와 해금 상태를 적용 */
	UFUNCTION(BlueprintCallable, Category = "RS|CharacterEntry")
	void SetupEntry(const FCharacterStaticData& InData, bool bInUnlocked);

	FORCEINLINE FName GetCharacterID() const { return CharData.CharacterID; }
	FORCEINLINE bool IsUnlocked() const { return bUnlocked; }

protected:
	/** Btn_Card OnClicked → OnEntryClickedDel 브로드캐스트 */
	UFUNCTION()
	void OnCardClicked();

	/** 현재 CharData / bUnlocked 상태를 위젯에 반영 */
	void UpdateDisplay();

public:
	/** 엔트리 클릭 시 부모 위젯(캐러셀/그리드)으로 CharID 전달 (LOCKED 시 발행 안 됨) */
	UPROPERTY(BlueprintAssignable, Category = "RS|CharacterEntry")
	FOnCharacterEntryClicked OnEntryClickedDel;

protected:
	/** 카드 전체 클릭 영역 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Card;
	/** 캐릭터 초상화 (LOCKED → 어두운 틴트) */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img_Portrait;
	/** 잠금 시각 레이어 — 자물쇠 아이콘 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UOverlay> Overlay_Locked;
	/** 등급 텍스트 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_Grade;
	/** 캐릭터 이름 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_Name;

	// -------------------------------------------------------------------------
	// 데이터 캐시 — SetupEntry에서 저장, UpdateDisplay에서 읽기
	// -------------------------------------------------------------------------
	UPROPERTY(BlueprintReadOnly, Category = "RS|CharacterEntry")
	FCharacterStaticData CharData;
	UPROPERTY(BlueprintReadOnly, Category = "RS|CharacterEntry")
	bool bUnlocked = false;
};
