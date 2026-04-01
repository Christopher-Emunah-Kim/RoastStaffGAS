// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "RSOutGamePlayerController.generated.h"

/**
 * ARSOutGamePlayerController
 *
 * OUTGAME 레벨 전용 PlayerController.
 * 로비/캐릭터선택/스테이지선택 페이지 전환과 UI 이벤트 처리를 담당
 */
UCLASS()
class ROASTSTAFFGAS_API ARSOutGamePlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:
	/** OUTGAME 로비 — 캐릭터 선택 페이지로 전환. WBP_Lobby 버튼에서 호출 */
	UFUNCTION(BlueprintCallable, Category = "RS|OutGame")
	void OnCharacterSelectClicked();
	/** OUTGAME 로비 — 스테이지 선택 페이지로 전환. WBP_Lobby 버튼에서 호출 */
	UFUNCTION(BlueprintCallable, Category = "RS|OutGame")
	void OnStageSelectClicked();
	/** OUTGAME 로비 — 설정 팝업 열기. WBP_Lobby 버튼에서 호출 */
	UFUNCTION(BlueprintCallable, Category = "RS|OutGame")
	void OnSettingClicked();
	
	/** 캐릭터 선택 완료 콜백 — 선택 데이터 저장 후 로비로 복귀 */
	UFUNCTION(BlueprintCallable, Category = "RS|OutGame")
	void OnCharacterSelected(FName CharID);
	/** 스테이지 선택 완료 콜백 — 해당 스테이지 레벨로 전환 */
	UFUNCTION(BlueprintCallable, Category = "RS|OutGame")
	void OnStageSelected(FName StageID);

private:
	/** OUTGAME 레벨 진입 시 PERSISTENT HUD + 첫 PAGE(로비) 오픈 */
	void OpenFirstWidget();
	
};
