// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/K_BaseCharacter.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/K_BaseAttributeSet.h"
#include "System/K_LoggingSystem.h"

AK_BaseCharacter::AK_BaseCharacter()
	: bASCInitialized(false), CharacterLevel(1)
{
	//ASC
	ASC = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("ASC"));

	//AttributeSet
	BaseAttributeSet = CreateDefaultSubobject<UK_BaseAttributeSet>(TEXT("BaseAttributeSet"));
	ASC->AddAttributeSetSubobject<UK_BaseAttributeSet>(BaseAttributeSet);
	
	//Pawn
	
	
	
}

void AK_BaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void AK_BaseCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	InitializeAbilitySystem();
	
	
}

void AK_BaseCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	MyPostInitailizeComponents();
	
	//AttributeSet Delegate Binding
	
	//DataTable Data Binding
}

void AK_BaseCharacter::InitializeAbilitySystem()
{
	//중복 초기화 방지
	if (bASCInitialized || !ASC)
	{
		KHS_WARN(TEXT("ASC is not valid or already initialized"));
	}
	
	//ASC 초기화
	ASC->InitAbilityActorInfo(this, this);
	
	//초기 어빌리티 부여
	for (const auto& eachAbility : InitialAbilities)
	{
		if (eachAbility)
		{
			FGameplayAbilitySpec spec(eachAbility, CharacterLevel, INDEX_NONE, this);
			ASC->GiveAbility(spec);
		}
	}
	
	bASCInitialized = true;
	
	KHS_INFO(TEXT("[%s] AbilitySystem initialized. Abilities granted : %d"), *GetName(), InitialAbilities.Num());
}

UAbilitySystemComponent* AK_BaseCharacter::GetAbilitySystemComponent() const
{
	return ASC;
}

float AK_BaseCharacter::GetHealth() const
{
	return BaseAttributeSet? BaseAttributeSet->GetHealth() : 0.f;
}

float AK_BaseCharacter::GetMaxHealth() const
{
	return BaseAttributeSet? BaseAttributeSet->GetMaxHealth() : 0.f;
}

float AK_BaseCharacter::GetMana() const
{
	return BaseAttributeSet? BaseAttributeSet->GetMana() : 0.f;
}

float AK_BaseCharacter::GetMaxMana() const
{
	return BaseAttributeSet? BaseAttributeSet->GetMaxMana() : 0.f;
}


