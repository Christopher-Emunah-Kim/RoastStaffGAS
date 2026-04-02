// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "UI/OutGame/RSCharacterSelectWidget.h"
#include "UI/OutGame/RSStageSelectWidget.h"
#include "RSOutGamePlayerController.generated.h"

/**
 * ARSOutGamePlayerController
 *
 * OUTGAME 레벨 전용 PlayerController.
 * UI 페이지 전환 요청을 수신 → SwitchPageUI 호출 후 반환된 위젯에 델리게이트 바인딩.
 * 바인딩은 페이지가 실제로 열리는 시점에 발생(레이지).
 */
UCLASS()
class ROASTSTAFFGAS_API ARSOutGamePlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;

public:
	/** LobbyWidget::Btn_CharacterSelect → 경유 호출. 캐릭터 선택 PAGE 열기 + 델리게이트 바인딩 */
	UFUNCTION(BlueprintCallable, Category = "RS|OutGame")
	void OnCharacterSelectClicked();

	/** CharacterSelectWidget::OnStageSelectRequestedDel 수신. 스테이지 선택 PAGE 열기 + 델리게이트 바인딩 */
	UFUNCTION(BlueprintCallable, Category = "RS|OutGame")
	void OnStageSelectClicked();

	/** 설정 팝업 열기 */
	UFUNCTION(BlueprintCallable, Category = "RS|OutGame")
	void OnSettingClicked();

	/** 캐릭터 선택 확정 콜백 — SGS에 마지막 선택 캐릭터 저장 */
	UFUNCTION()
	void OnCharacterSelected(FName CharID);

	/** 스테이지 선택 확정 콜백 — SGS 저장 후 스테이지 레벨로 전환 */
	UFUNCTION()
	void OnStageSelected(FName StageID);

private:
	/** OUTGAME 레벨 진입 시 PERSISTENT HUD + 첫 PAGE(로비) 오픈 */
	void OpenFirstWidget();

	// ── 위젯 캐시 — GC 추적 필수 ──────────────────────────────────────────────
	UPROPERTY()
	TObjectPtr<URSCharacterSelectWidget> CachedCharSelectWidget;

	UPROPERTY()
	TObjectPtr<URSStageSelectWidget> CachedStageSelectWidget;
};
