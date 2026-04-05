// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/RSBaseWidget.h"
#include "RSStageResultWidget.generated.h"

class UTextBlock;
class UButton;

/**
 * 결과 확인 버튼 클릭 시 발생하는 델리게이트
 * RSGameMode에서 구독하여 OUTGAME 복귀 처리
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnConfirmClickedDel);

/**
 * URSStageResultWidget
 *
 * 스테이지 클리어/실패 결과 표시 UI
 * - 클리어 상태, 플레이 시간, 처치 수 표시
 * - 최고 기록 (최장 생존, 최다 처치) 표시
 * - 확인 버튼 클릭 → OUTGAME 복귀 트리거
 */
UCLASS()
class ROASTSTAFFGAS_API URSStageResultWidget : public URSBaseWidget
{
	GENERATED_BODY()

public:
	/** 결과 확인 버튼 클릭 시 브로드캐스트 */
	UPROPERTY(BlueprintAssignable, Category = "Stage Result Events")
	FOnConfirmClickedDel OnConfirmClickedDel;

protected:
	virtual void NativeOnInitialized() override;

public:
	virtual void RefreshUI() override;

	/** 결과 데이터 설정 (RSGameMode에서 호출) */
	void SetResultData(bool bInCleared, float PlayTime, int32 KillCount, float BestTime, int32 BestKill, const FString& StageName);

private:
	UFUNCTION()
	void OnConfirmButtonClicked();

protected:
	// BindWidget 프로퍼티 (WBP_StageResultWidget에서 바인딩)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_StageTitle;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_ClearStatus;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_PlayTime;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_KillCount;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_BestTime;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_BestKill;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Confirm;

private:
	// 결과 데이터 캐시
	bool bIsCleared = false;
	float PlayTimeSeconds = 0.f;
	int32 TotalKillCount = 0;
	float BestTimeSeconds = 0.f;
	int32 BestKillCount = 0;
	FString StageDisplayName;
};
