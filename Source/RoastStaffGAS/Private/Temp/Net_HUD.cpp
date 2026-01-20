// Fill out your copyright notice in the Description page of Project Settings.


#include "Temp/Net_HUD.h"

#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Temp/K_NetAttributeSet.h"
#include "Temp/Net_GameState.h"
#include "Temp/Net_PlayerState.h"

void UNet_HUD::NativeConstruct()
{
	NetGameState = Cast<ANet_GameState>(UGameplayStatics::GetGameState(this));
	check(NetGameState);
}

void UNet_HUD::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	if (!NetGameState)
	{
		return;
	}
	
	UpdateTimer();
	UpdateScores();
}

void UNet_HUD::UpdateTimer()
{
	int32 time = NetGameState->GetRemainingTime();
	
	if (txt_Timer)
	{
		txt_Timer->SetText(FText::FromString(FString::Printf(TEXT("Time : %d"),time)));
	}
	
	if (LastRemainingTime > 0 && time == 0)
	{
		//TODO GameState Gameover RPC콜백 연결
	}
	
	LastRemainingTime = time;
}

void UNet_HUD::UpdateScores()
{
	const auto& playerArray = NetGameState->PlayerArray;
	
	for (int32 i = 0; i < FMath::Min(2, playerArray.Num()); ++i)
	{
		auto* ps = Cast<ANet_PlayerState>(playerArray[i]);
		if (!ensureMsgf(ps, TEXT("Failed to cast PlayerState")))
		{
			continue;
		}
		auto* attrs = ps->GetNetAttributeSet();
		if (!ensureMsgf(attrs, TEXT("Failed to get NetAttributeSet")))
		{
			continue;
		}
		
		float score = attrs->GetItemCount();
		FString scoreText = FString::Printf(TEXT("Player %d : Score : %.0f"),i +1, score);
		
		if (i == 0 && txt_Player1Score)
		{
			txt_Player1Score->SetText(FText::FromString(scoreText));
		}
		else if (i == 1 && txt_Player2Score)
		{
			txt_Player2Score->SetText(FText::FromString(scoreText));
		}
	}
}
