// Fill out your copyright notice in the Description page of Project Settings.


#include "Temp/Net_PlayerCharacter.h"
#include "RoastStaffGAS.h"
#include "AbilitySystemComponent.h"
#include "System/K_LoggingSystem.h"
#include "Temp/K_NetAttributeSet.h"
#include "Temp/Net_PlayerState.h"

ANet_PlayerCharacter::ANet_PlayerCharacter()
{
	
}

void ANet_PlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	InitializeAbilitySystem();
	KHS_INFO(TEXT(" PossessedBy called on Server"));
}

void ANet_PlayerCharacter::InitializeAbilitySystem()
{
	Super::InitializeAbilitySystem();
	
	ANet_PlayerState* ps = Cast<ANet_PlayerState>(GetKPlayerState());
	if (!ps)
	{
		KHS_WARN(TEXT("NetPlayerState not found, cannot initialize abilities"));
		return;
	}
	
	ps->InitializeAbilities(this);
	
	UAbilitySystemComponent* AbilitySystem = ps->GetAbilitySystemComponent();
	UK_NetAttributeSet* AttributeSet = ps->GetNetAttributeSet();
	
	if (AbilitySystem && AttributeSet)
	{
		AbilitySystem->SetNumericAttributeBase(AttributeSet->GetItemCountAttribute(), 0.f);
	}
}

void ANet_PlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	
	InitializeAbilitySystem();
	KHS_INFO(TEXT(" OnRep_PlayerState called on Client"));
}

UK_NetAttributeSet* ANet_PlayerCharacter::GetNetAttributeSet() const
{
	ANet_PlayerState* ps = Cast<ANet_PlayerState>(GetKPlayerState());
	if (!ps)
	{
		KHS_WARN(TEXT(" NetPlayerState not found"));
		return nullptr;
	}
	
	UK_NetAttributeSet* AttributeSet = ps->GetNetAttributeSet();
	
	if (!AttributeSet)
	{
		KHS_WARN(TEXT("Invalid AttributeSet"));
		return nullptr;
	}
	
	return AttributeSet;
}

void ANet_PlayerCharacter::AddPickUp_Implementation()
{
	//서버가 아니면 리턴.
	if (GetLocalRole() != ROLE_Authority)
	{
		return;
	}
	
	UAbilitySystemComponent* abilityComp = GetAbilitySystemComponent();
	if (!ensureMsgf(abilityComp, TEXT("Invalid ASC")))
	{
		return;
	}
	
	if (!ensureMsgf(PickUpEffect, TEXT("PickUpEffect is not set on BP")))
	{
		return;
	}
	FGameplayEffectContextHandle contextHandle = abilityComp->MakeEffectContext();
	contextHandle.AddSourceObject(this);
	AActor* owner = abilityComp->GetAvatarActor();
	if (owner)
	{
		contextHandle.AddInstigator(owner, this); //플레이어가 가해자.
	}
	else
	{
		KHS_WARN(TEXT(" Invalid AvatarActor for Instigator "));
	}
	
	//Create Effect Spec : GE의 인스턴스 생성
	FGameplayEffectSpecHandle specHandle = abilityComp->MakeOutgoingSpec(PickUpEffect, 1.f, contextHandle);
	if (!specHandle.IsValid())
	{
		KHS_WARN(TEXT("Failed to make outgoing spec for effect : %s"), *PickUpEffect->GetName());
		return;
	}
	
	//AcitvateHandle값을 반환하지만 지금은 쓸곳없으니 무시
	(void)abilityComp->ApplyGameplayEffectSpecToSelf(*specHandle.Data.Get());
	
	UK_NetAttributeSet* attrs = GetNetAttributeSet();
	KHS_SCREEN_INFO(TEXT("[AddPickup] After GE - ItemCount: %.1f"), 
		attrs ? attrs->GetItemCount() : -1.f);
}
