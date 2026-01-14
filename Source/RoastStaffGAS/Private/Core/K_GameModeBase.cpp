// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/K_GameModeBase.h"
#include "RoastStaffGAS.h"
#include "System/K_LoggingSystem.h"
#include "GameplayTags/K_GameplayTags.h"

void AK_GameModeBase::BeginPlay()
{
	Super::BeginPlay();
	
	TestGasModules();
}

void AK_GameModeBase::TestGasModules()
{
	// AbilitySystemComponent 클래스 존재 확인
	KHS_INFO(TEXT("========================================"));
	KHS_INFO(TEXT("GAS Module Connection Test"));
	KHS_INFO(TEXT("========================================"));
    
	if (UAbilitySystemComponent::StaticClass())
	{
		KHS_INFO(TEXT("[OK] AbilitySystemComponent is available"));
	}
    
	if (UGameplayEffect::StaticClass())
	{
		KHS_INFO(TEXT("[OK] GameplayEffect is available"));
	}
    
	if (UAttributeSet::StaticClass())
	{
		KHS_INFO(TEXT("[OK] AttributeSet is available"));
	}
    
	if (UGameplayAbility::StaticClass())
	{
		KHS_INFO(TEXT("[OK] GameplayAbility is available"));
	}
    
	KHS_INFO(TEXT("========================================"));
	KHS_INFO(TEXT("GAS Setup Complete! "));
	KHS_INFO(TEXT("========================================"));
	
	KHS_INFO(TEXT(""));
	KHS_INFO(TEXT("========================================"));
	KHS_INFO(TEXT("GameplayTag Registration Test"));
	KHS_INFO(TEXT("========================================"));
	
	auto TestTag = [](const FGameplayTag& tag, const TCHAR* description)
	{
		if (tag.IsValid())
		{
			KHS_INFO(TEXT("[OK] %s : %s"), description, *tag.ToString());
		}
		else
		{
			KHS_ERROR(TEXT("[FAIL] %s is not valid"), description);
		}
	};
	
	TestTag(KTags::Ability_Skill_Fireball, TEXT("Fireball Ability"));
	TestTag(KTags::Ability_Skill_Fireball_Cooldown, TEXT("Fireball Ability Cooldown"));
	TestTag(KTags::State_Debuff_Burning, TEXT("Debuff Burning"));
	TestTag(KTags::Data_Damage, TEXT("Data Damage"));
	TestTag(KTags::GameplayCue_Combat_Fireball_Impact, TEXT("GC_Combat_Fireball_Impact"));
	
	FGameplayTag debuffParent = FGameplayTag::RequestGameplayTag(FName("State.Debuff"));
	if (KTags::State_Debuff_Burning.GetTag().MatchesTag(debuffParent))
	{
		KHS_INFO(TEXT("[OK] Hierarchy Test : Burning matches State.Debuff"));
	}
	
	KHS_INFO(TEXT("========================================"));
	KHS_INFO(TEXT("GameplayTag Registration Complete! "));
	KHS_INFO(TEXT("========================================"));
}
