// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/Transition/RSTransitionPlayerController.h"

void ARSTransitionPlayerController::BeginPlay()
{
	Super::BeginPlay();

	SetShowMouseCursor(false);

	FInputModeUIOnly UIOnlyMode;
	SetInputMode(UIOnlyMode);

	// LOADING 위젯 오픈은 ARSTransitionGameMode::BeginPlay가 담당
}
