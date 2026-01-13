// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/K_BaseCharacter.h"
#include "AbilitySystemComponent.h"

// Sets default values
AK_BaseCharacter::AK_BaseCharacter()
{
	//ASC
	ASC = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("ASC"));

	//AttributeSet
	
	//Pawn
	
	
	
	StartLevel = 1;
}

// Called when the game starts or when spawned
void AK_BaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void AK_BaseCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	for (const auto& eachAbility : InitialAbilities)
	{
		ASC->GiveAbility(FGameplayAbilitySpec(eachAbility));
	}
}

void AK_BaseCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	MyPostInitailizeComponents();
	
	//AttributeSet Delegate Binding
	
	//DataTable Data Binding
}

UAbilitySystemComponent* AK_BaseCharacter::GetAbilitySystemComponent() const
{
	return ASC;
}


