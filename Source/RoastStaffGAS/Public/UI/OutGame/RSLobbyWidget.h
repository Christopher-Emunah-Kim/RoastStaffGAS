// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/RSBaseWidget.h"
#include "RSLobbyWidget.generated.h"

class UButton;

/**
 * URSLobbyWidget
 *
 * OutGame 로비 PAGE 위젯.
 * 캐릭터 선택 / 스테이지 선택 / 설정 화면으로 진입하는 허브.
 *
 * [WBP 필수 위젯 이름 목록]
 *   Btn_CharacterSelect — UButton, 캐릭터 선택 PAGE로 전환
 *   Btn_StageSelect     — UButton, 스테이지 선택 PAGE로 전환
 *   Btn_Settings        — UButton, 설정 팝업 열기
 */
UCLASS()
class ROASTSTAFFGAS_API URSLobbyWidget : public URSBaseWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;

	UFUNCTION() void OnCharacterSelectClicked();
	UFUNCTION() void OnStageSelectClicked();
	UFUNCTION() void OnSettingsClicked();

protected:
	// BindWidget — WBP에서 아래 이름과 정확히 일치하는 위젯 생성 필수
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_CharacterSelect;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_StageSelect;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Settings;
};
