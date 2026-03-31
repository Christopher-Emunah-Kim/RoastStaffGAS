// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "RSIntroPlayerController.generated.h"

/**
 * ARSIntroPlayerController
 *
 * INTRO 레벨 전용 PlayerController.
 *
 */
UCLASS()
class ROASTSTAFFGAS_API ARSIntroPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:
	/** WBP_Intro FadeAnim 완료 후 BindToAnimationEvent(FadeAnim, AnimFinishedDel, EWidgetAnimationEvent::Finished);에 바인딩 연결. */
	UFUNCTION(BlueprintCallable, Category = "RS|Intro")
	void OpenTitleScreen();

	/** WBP_Title '게임 시작' 버튼 바인딩 이벤트 콜백 함수에서 호출 */
	UFUNCTION(BlueprintCallable, Category = "RS|Intro")
	void OnStartGameClicked();

private:
	void OpenFirstWidget();
};
