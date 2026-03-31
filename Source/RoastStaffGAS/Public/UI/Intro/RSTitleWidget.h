// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/RSBaseWidget.h"
#include "RSTitleWidget.generated.h"

class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStartGameRequested);

/**
 * URSTitleWidget
 *
 * INTRO 레벨 타이틀 화면 위젯.
 */
UCLASS()
class ROASTSTAFFGAS_API URSTitleWidget : public URSBaseWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

private:
	UFUNCTION()
	void HandleStartClicked();

public:
	/** ARSIntroPlayerController::OnStartGameClicked에 바인딩 */
	UPROPERTY(BlueprintAssignable, Category = "RS|Title")
	FOnStartGameRequested OnStartGameRequestedDel;
	
private:
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Start;
};
