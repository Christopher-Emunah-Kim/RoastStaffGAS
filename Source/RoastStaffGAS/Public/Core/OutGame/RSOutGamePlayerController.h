// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Character/Player/LobbyCharacterActor.h"
#include "UI/OutGame/RSStageSelectWidget.h"
#include "RSOutGamePlayerController.generated.h"

class URSLobbyWidget;

/**
 * ARSOutGamePlayerController
 *
 * OUTGAME 레벨 전용 PlayerController.
 * 로비: 3D LobbyCharacterActor 클릭 수신 → 카메라 블렌드 + 정보 패널 오픈.
 * 스테이지 선택: SwitchPageUI 후 OnStageSelected 델리게이트 바인딩.
 * WBP 버튼(OnConfirmClicked / OnBackClicked)은 BlueprintCallable로 직접 호출.
 */
UCLASS()
class ROASTSTAFFGAS_API ARSOutGamePlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	virtual void PlayerTick(float DeltaTime) override;

public:
	/** 로비 3D 캐릭터 클릭 콜백 — ALobbyCharacterActor::OnCharacterClickedDel 수신 */
	UFUNCTION()
	void OnLobbyCharacterClicked(FName CharID);
	/** 캐릭터 정보 패널 확정 버튼 — WBP_LobbyCharInfoPanel에서 직접 호출 */
	UFUNCTION(BlueprintCallable, Category = "RS|OutGame")
	void OnConfirmClicked();
	/** 캐릭터 정보 패널 뒤로가기 버튼 — WBP_LobbyCharInfoPanel에서 직접 호출 */
	UFUNCTION(BlueprintCallable, Category = "RS|OutGame")
	void OnBackClicked();

	/** 설정 팝업 열기 */
	UFUNCTION(BlueprintCallable, Category = "RS|OutGame")
	void OnSettingClicked();
	/** 스테이지 선택 확정 콜백 — URSStageSelectWidget::OnStageSelectedDel 수신 */
	UFUNCTION()
	void OnStageSelected(FName StageID);

private:
	/** OUTGAME 레벨 진입 시 PERSISTENT HUD + 로비 PAGE 오픈 */
	void OpenFirstWidget();
	/** 레벨 배치 LobbyCharacterActor 탐색 + 델리게이트 바인딩 */
	void InitLobbyActors();
	/** 전체 조망 카메라로 블렌드 복귀 */
	void SetViewToOverview();
	/** PlayerTick에서 매 프레임 커서 위치 트레이스 → 아웃라인 수동 제어 */
	void UpdateLobbyHover();

	// ── 위젯 캐시 — GC 추적 필수 ──────────────────────────────────────────────
	UPROPERTY()
	TObjectPtr<URSLobbyWidget> CachedLobbyWidget;
	/** 현재 호버 중인 LobbyCharacterActor — 프레임 간 비교용 */
	UPROPERTY()
	TObjectPtr<ALobbyCharacterActor> CurrentHoveredLobbyChar;
	UPROPERTY()
	TObjectPtr<URSStageSelectWidget> CachedStageSelectWidget;
	// ── 로비 Actor 캐시 ────────────────────────────────────────────────────────
	UPROPERTY()
	TArray<TObjectPtr<ALobbyCharacterActor>> LobbyCharacters;
	/** "LobbyOverviewCamera" 태그 Actor — BeginPlay에서 탐색 */
	UPROPERTY()
	TObjectPtr<AActor> OverviewCameraRef;

	/** 전체 조망 카메라 복귀 블렌드 시간 (초) */
	UPROPERTY(EditDefaultsOnly, Category = "Lobby")
	float OverviewBlendTime = 0.8f;
};