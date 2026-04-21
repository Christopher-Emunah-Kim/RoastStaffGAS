// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/RSBaseWidget.h"
#include "LobbyCharInfoPanel.generated.h"

class UTextBlock;
class UImage;
class UHorizontalBox;
class UButton;
class ARSOutGamePlayerController;
struct FCharacterSkillStaticData;

/**
 * ULobbyCharInfoPanel
 *
 * 로비에서 캐릭터 클릭 후 우측에 표시되는 정보 패널.
 * NativeConstruct에서 RDS::GetSelectedCharacterID()로 CharID를 읽어 자동 Populate.
 * Btn_Confirm / Btn_Back → PC::OnConfirmClicked / OnBackClicked 직접 호출 (WBP에서 처리).
 */
UCLASS()
class ROASTSTAFFGAS_API ULobbyCharInfoPanel : public URSBaseWidget
{
	GENERATED_BODY()

public:
	/** 패널 표시 + CharID로 정보 채우기 */
	void Show(FName CharID);
	/** 패널 숨기기 (Collapsed) */
	void Hide();

protected:
	virtual void NativeOnInitialized() override;

private:
	/** CharID 기준으로 이름 + 초상화 + 스킬 아이콘 전체 세팅 */
	void PopulateWithCharacter(FName CharID);
	/** SkillIconContainer에 스킬 아이콘 UImage 동적 생성 */
	void PopulateSkillIcons(const TArray<FCharacterSkillStaticData>& Skills);
	/** Btn_Confirm 클릭 — PC::OnConfirmClicked 위임 */
	UFUNCTION()
	void OnConfirmButtonClicked();
	/** Btn_Back 클릭 — PC::OnBackClicked 위임 */
	UFUNCTION()
	void OnBackButtonClicked();

	// ── BindWidget ──────────────────────────────────────────────────────────
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_CharName;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img_Portrait;
	/** 스킬 아이콘 6개를 동적으로 채울 컨테이너 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> SkillIconContainer;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Confirm;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Back;

	/** 동적 생성 아이콘 크기 (px) — WBP DefaultsOnly에서 오버라이드 가능 */
	UPROPERTY(EditDefaultsOnly, Category = "Lobby")
	float SkillIconSize = 48.f;
	/** 아이콘 간 좌우 패딩 (px) */
	UPROPERTY(EditDefaultsOnly, Category = "Lobby")
	float SkillIconPadding = 4.f;
};
