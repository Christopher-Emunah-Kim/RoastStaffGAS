// Fill out your copyright notice in the Description page of Project Settings.


#include "Temp/Net_GameState.h"
#include "Net/UnrealNetwork.h"
#include "RoastStaffGAS.h"

ANet_GameState::ANet_GameState()
{
	RemainingTime = 60;
}


void ANet_GameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ANet_GameState, RemainingTime);
}

void ANet_GameState::OnRep_RemainingTime()
{
	//TODO 이후 HUD 위젯에서 구독해서 갱신
	KHS_INFO(TEXT("[Net_GameState] RemainingTime Updated : %d"), RemainingTime);
}

void ANet_GameState::GameOverRPC_Implementation(int32 WinnerPlayerIdx)
{
	KHS_INFO(TEXT("[Net_GameState] Game Over! Winner: Player %d"), WinnerPlayerIdx);
    
	//TODO 이후 UI에 승패 표시
}
