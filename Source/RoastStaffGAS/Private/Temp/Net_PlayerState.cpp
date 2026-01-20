// Fill out your copyright notice in the Description page of Project Settings.


#include "Temp/Net_PlayerState.h"
#include "AbilitySystemComponent.h"
#include "System/K_LoggingSystem.h"
#include "Temp/K_NetAttributeSet.h"

ANet_PlayerState::ANet_PlayerState()
{
	if (ASC)
	{
		ASC->SetIsReplicated(true);
		ASC->SetReplicationMode(EGameplayEffectReplicationMode::Full);
	}
	
	NetAttributeSet = CreateDefaultSubobject<UK_NetAttributeSet>(TEXT("NetAttributeSet"));
	if (ASC)
	{
		ASC->AddAttributeSetSubobject<UK_NetAttributeSet>(NetAttributeSet);
		KHS_INFO(TEXT("[NetPlayerState] NetAttributeSet added to ASC"));
	}
}
