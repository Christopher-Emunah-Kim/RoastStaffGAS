// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "RSTransitionPlayerController.generated.h"

/**
 * ARSTransitionPlayerController
 *
 * TRANSITION 레벨 전용 PlayerController.
 * BeginPlay에서 LOADING 위젯을 열어 로딩 화면을 표시한다.
 * 실제 로딩 진행은 ARSTransitionGameMode가 담당.
 */
UCLASS()
class ROASTSTAFFGAS_API ARSTransitionPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
};
