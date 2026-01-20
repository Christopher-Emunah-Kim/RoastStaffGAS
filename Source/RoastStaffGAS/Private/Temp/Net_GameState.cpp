// Fill out your copyright notice in the Description page of Project Settings.


#include "Temp/Net_GameState.h"
#include "Net/UnrealNetwork.h"
#include "RoastStaffGAS.h"
#include "Character/K_PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Temp/Net_HUD.h"

ANet_GameState::ANet_GameState()
{
	RemainingTime = 60;
}


void ANet_GameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ANet_GameState, RemainingTime);
}

void ANet_GameState::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);
	
	if (PlayerState)
	{
		KHS_INFO(TEXT("New player added : %s"), *PlayerState->GetName());
		OnPlayerAdded.Broadcast(PlayerState);
	}
}

void ANet_GameState::OnRep_RemainingTime()
{
	OnRemainingTimeChanged.Broadcast(RemainingTime);
	//KHS_INFO(TEXT("RemainingTime Updated : %d"), RemainingTime);
}

void ANet_GameState::SetRemainingTime(int32 NewTime)
{
	//서버의 경우 직접 RemainingTime 변경
	if (RemainingTime == NewTime)
	{
		return;
	}
	
	RemainingTime = NewTime;
		
	OnRemainingTimeChanged.Broadcast(RemainingTime);
}

void ANet_GameState::GameOverRPC_Implementation(int32 WinnerPlayerIdx)
{
	KHS_INFO(TEXT("Game Over! Winner: Player %d"), WinnerPlayerIdx);
    
	AK_PlayerController* pc = CastChecked<AK_PlayerController>(GetWorld()->GetFirstPlayerController());
	UNet_HUD* HUD = CastChecked<UNet_HUD>(pc->GetNetHUD());
	HUD->ShowGameResult(WinnerPlayerIdx);
}
