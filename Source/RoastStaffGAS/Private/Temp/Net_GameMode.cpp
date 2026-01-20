// Fill out your copyright notice in the Description page of Project Settings.


#include "Temp/Net_GameMode.h"
#include "Temp/Net_GameState.h"
#include "RoastStaffGAS.h"
#include "Temp/K_NetAttributeSet.h"
#include "Temp/Net_PlayerState.h"

ANet_GameMode::ANet_GameMode()
	: TimeAccumulator(0.f), bGameEnded(false)
{
	PrimaryActorTick.bCanEverTick = true;
	
	GameStateClass = ANet_GameState::StaticClass();
}

void ANet_GameMode::BeginPlay()
{
	Super::BeginPlay();
	
	NetGameState = Cast<ANet_GameState>(GameState);
	
	if (NetGameState)
	{
		KHS_INFO(TEXT(" GameState Initialized. Starting 60 sec timer"));
	}
}

void ANet_GameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (bGameEnded || !NetGameState)
	{
		return;
	}
	
	TimeAccumulator += DeltaTime;
	
	if (TimeAccumulator >= 1.f)
	{
		TimeAccumulator -= 1.f;
		
		int32 Time = NetGameState->GetRemainingTime() - 1;
		Time = FMath::Max(0, Time);
		
		NetGameState->SetRemainingTime(Time);
		
		if (Time <= 0)
		{
			EndGame();
		}
	}
	
}

int32 ANet_GameMode::DetermineWinner()
{
	if (!ensureMsgf(NetGameState, TEXT("Invalid NetGameState")))
	{
		return -1;
	}
	
	const TArray<APlayerState*>& playerArray = NetGameState->PlayerArray;
	
	if (playerArray.Num() < 2)
	{
		KHS_WARN(TEXT(" Not enough player for multiplay"));
		return -1;
	}
	
	int32 winnderIdx = 0;
	float maxItemCount = 1.f;
	
	for (int32 i = 0; i < playerArray.Num(); i++)
	{
		ANet_PlayerState* ps = CastChecked<ANet_PlayerState>(playerArray[i]);
		UK_NetAttributeSet* netAttributeSet = ps->GetNetAttributeSet();
		ensure(netAttributeSet);
		
		float itemCount = netAttributeSet->GetItemCount();
		KHS_INFO(TEXT("PlayerId %d, ItemCount %.1f"), i, itemCount);
		
		if (itemCount > maxItemCount)
		{
			maxItemCount = itemCount;
			winnderIdx = i;
		}
	}

	return winnderIdx;
}

void ANet_GameMode::EndGame()
{
	bGameEnded = true;
	
	int32 winnerIdx = DetermineWinner();
	
	KHS_INFO(TEXT(" GameOver! Winner Index: %d"), winnerIdx);
	
	if (NetGameState)
	{
		NetGameState->GameOverRPC(winnerIdx);
	}
}
