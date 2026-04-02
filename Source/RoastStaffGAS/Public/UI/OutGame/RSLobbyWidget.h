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
 * 캐릭터 선택(→ 스테이지 선택) / 설정 화면 진입 허브.
 * 스테이지 선택은 캐릭터 선택 완료 후 CharacterSelectWidget에서 진행.
 *
 */
UCLASS()
class ROASTSTAFFGAS_API URSLobbyWidget : public URSBaseWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;

	UFUNCTION() void OnCharacterSelectClicked();
	UFUNCTION() void OnSettingsClicked();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_CharacterSelect;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Settings;
};
