// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/K_BaseCharacter.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/K_BaseAttributeSet.h"
#include "System/K_LoggingSystem.h"

AK_BaseCharacter::AK_BaseCharacter()
	: CharacterLevel(1), bASCInitialized(false)
{
	//ASC  - NPC / AI 사용
	//player에도 이 컴포넌트가 생성되나 미사용.
	//조건부 생성하는 방법도 있으나 단순함을 위해 항상 생성.
	ASC = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("ASC"));
	ASC->SetIsReplicated(true);
	
	//AttributeSet 생성 및 ASC등록
	BaseAttributeSet = CreateDefaultSubobject<UK_BaseAttributeSet>(TEXT("BaseAttributeSet"));
	ASC->AddAttributeSetSubobject<UK_BaseAttributeSet>(BaseAttributeSet);
	
	
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
		return;
	}
	
	//ASC 초기화  - NPC / AI 사용
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


