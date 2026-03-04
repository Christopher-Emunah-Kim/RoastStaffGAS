// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/K_GameModeBase.h"
#include "RoastStaffGAS.h"
#include "GAS/Attributes/K_BaseAttributeSet.h"
#include "GAS/Tags/K_GameplayTags.h"
#include "System/LoggingSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Character/K_PlayerCharacter.h"

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
	
	TestTag(KTags::Ability_Skill_Fireball1, TEXT("Fireball Ability"));
	TestTag(KTags::Ability_Skill_Fireball_Cooldown1, TEXT("Fireball Ability Cooldown"));
	TestTag(KTags::State_Debuff_Burn001, TEXT("Debuff Burning"));
	TestTag(KTags::Data_Damage001, TEXT("Data Damage"));
	TestTag(KTags::GameplayCue_Combat_Fireball_Impact001, TEXT("GC_Combat_Fireball_Impact"));
	
	FGameplayTag debuffParent = FGameplayTag::RequestGameplayTag(FName("State.Debuff"));
	if (KTags::State_Debuff_Burn001.GetTag().MatchesTag(debuffParent))
	{
		KHS_INFO(TEXT("[OK] Hierarchy Test : Burning matches State.Debuff"));
	}
	
	KHS_INFO(TEXT("========================================"));
	KHS_INFO(TEXT("GameplayTag Registration Complete! "));
	KHS_INFO(TEXT("========================================"));
	
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
	{
		APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (!PC) return;
        
		AK_PlayerCharacter* PlayerChar = Cast<AK_PlayerCharacter>(PC->GetPawn());
		if (!PlayerChar) return;
        
		KHS_INFO(TEXT(""));
		KHS_INFO(TEXT("========================================"));
		KHS_INFO(TEXT("AttributeSet Test"));
		KHS_INFO(TEXT("========================================"));
        
		UAbilitySystemComponent* ASC = PlayerChar->GetAbilitySystemComponent();
		if (ASC)
		{
			KHS_INFO(TEXT("[OK] ASC is valid"));
		}
        
		UK_BaseAttributeSet* Attrs = PlayerChar->GetAttributeSet();
		if (Attrs)
		{
			KHS_INFO(TEXT("[OK] AttributeSet is valid"));
			KHS_INFO(TEXT("    Health: %.1f / %.1f"), Attrs->GetHealth(), Attrs->GetMaxHealth());
			KHS_INFO(TEXT("    Mana: %.1f / %.1f"), Attrs->GetMana(), Attrs->GetMaxMana());
		}
        
		KHS_INFO(TEXT("========================================"));
		KHS_INFO(TEXT("AttributeSet Test Complete!"));
		KHS_INFO(TEXT("========================================"));
        
	}, 1.0f, false);
}
