// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/RSBaseWidget.h"
#include "RSLobbyWidget.generated.h"

class UButton;
class ULobbyCharInfoPanel;

/**
 * URSLobbyWidget
 *
 * OutGame 로비 PAGE 위젯.
 * v2.0: 캐릭터 선택은 3D LobbyCharacterActor 클릭으로 처리 — Btn_CharacterSelect 제거.
 * Panel_CharInfo를 BindWidget으로 내부 보유 — 전체화면 오버레이 없이 우측 패널 토글.
 */
UCLASS()
class ROASTSTAFFGAS_API URSLobbyWidget : public URSBaseWidget
{
	GENERATED_BODY()

public:
	/** 캐릭터 클릭 시 우측 패널 표시 */
	void ShowCharInfo(FName CharID);
	/** 뒤로가기 시 우측 패널 숨기기 */
	void HideCharInfo();

protected:
	virtual void NativeOnInitialized() override;

private:
	/** 우측 캐릭터 정보 패널 — WBP에서 배치. 기본 Collapsed */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<ULobbyCharInfoPanel> Panel_CharInfo;
};
