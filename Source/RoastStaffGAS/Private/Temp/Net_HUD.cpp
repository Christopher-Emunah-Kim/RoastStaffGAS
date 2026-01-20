// Fill out your copyright notice in the Description page of Project Settings.


#include "Temp/Net_HUD.h"
#include "Temp/K_NetAttributeSet.h"
#include "Temp/Net_GameState.h"
#include "Temp/Net_PlayerState.h"
#include "RoastStaffGAS.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

void UNet_HUD::NativeConstruct()
{
	NetGameState = Cast<ANet_GameState>(UGameplayStatics::GetGameState(this));
	check(NetGameState);
	
	FTimerHandle DelayHandle;
	GetWorld()->GetTimerManager().SetTimer(DelayHandle, [this]()
	{
		BindToGameState();
		OnTimeChanged(NetGameState->GetRemainingTime());
		UpdatePlayerScore();
	}, 0.5f, false);
}

void UNet_HUD::NativeDestruct()
{
	UnbindFromGameState();
	Super::NativeDestruct();
}

void UNet_HUD::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	if (!NetGameState)
	{
		return;
	}
}

void UNet_HUD::OnTimeChanged(int32 NewTime)
{
	if (txt_Timer)
	{
		txt_Timer->SetText(FText::FromString(FString::Printf(TEXT("Time: %d"), NewTime)));
	}
}

void UNet_HUD::OnPlayerScoreChanged(const FOnAttributeChangeData& Data)
{
	KHS_INFO(TEXT("Score changed: %.1f -> %.1f"), Data.OldValue, Data.NewValue);
	UpdatePlayerScore();
}

void UNet_HUD::BindToGameState()
{
	if (!ensureMsgf(NetGameState, TEXT("Failed to get NetGameState")))
	{
		return;
	}
	
	//타이머 이벤트 구독
	NetGameState->OnRemainingTimeChanged.AddDynamic(this, &UNet_HUD::OnTimeChanged);
	
	//플레이어별 item count Attribute 변경 감지 구독
	for (APlayerState* ps : NetGameState->PlayerArray)
	{
		ANet_PlayerState* netPS = Cast<ANet_PlayerState>(ps);
		if (!ensureMsgf(netPS, TEXT("Failed to get NetPlayerState")))
		{
			continue;
		}
		
		UAbilitySystemComponent* ASC = netPS->GetAbilitySystemComponent();
		if (!ensureMsgf(ASC, TEXT("Failed to get ASC")))
		{
			continue;
		}
		
		UK_NetAttributeSet* attrs = netPS->GetNetAttributeSet();
		if (!ensureMsgf(attrs, TEXT("Failed to get NetAttributeSet")))
		{
			continue;
		}
		
		ASC->GetGameplayAttributeValueChangeDelegate(attrs->GetItemCountAttribute()).AddUObject(this, &UNet_HUD::OnPlayerScoreChanged);
	}
	
	KHS_INFO(TEXT("Bound to GameState events"));
}

void UNet_HUD::UnbindFromGameState()
{
	if (!ensureMsgf(NetGameState, TEXT("Failed to get NetGameState")))
	{
		return;
	}
	
	//타이머 이벤트 구독 해제
	NetGameState->OnRemainingTimeChanged.RemoveDynamic(this, &UNet_HUD::OnTimeChanged);
	
	//플레이어별 item count Attribute 변경 감지 구독 해제
	for (APlayerState* ps : NetGameState->PlayerArray)
	{
		ANet_PlayerState* netPS = Cast<ANet_PlayerState>(ps);
		if (!ensureMsgf(netPS, TEXT("Failed to get NetPlayerState")))
		{
			continue;
		}
		
		UAbilitySystemComponent* ASC = netPS->GetAbilitySystemComponent();
		if (!ensureMsgf(ASC, TEXT("Failed to get ASC")))
		{
			continue;
		}
		
		UK_NetAttributeSet* attrs = netPS->GetNetAttributeSet();
		if (!ensureMsgf(attrs, TEXT("Failed to get NetAttributeSet")))
		{
			continue;
		}
		
		ASC->GetGameplayAttributeValueChangeDelegate(attrs->GetItemCountAttribute()).RemoveAll(this);
	}
	
	KHS_INFO(TEXT("Bound to GameState events"));
}

void UNet_HUD::UpdatePlayerScore()
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

