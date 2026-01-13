// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "K_BaseCharacter.generated.h"

class UGameplayAbility;
class UAbilitySystemComponent;


UCLASS()
class ROASTSTAFFGAS_API AK_BaseCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AK_BaseCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void PostInitializeComponents() override;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
public:
	UFUNCTION(BlueprintImplementableEvent)
	void MyPostInitailizeComponents();

protected:
	/* //////////////////////
	 * GAS Contents
	 *///////////////////////
	
	//ASC
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AM|GAS")
	TObjectPtr<class UAbilitySystemComponent> ASC;
	
	//Game Abilities
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AM|GAS")
	TArray<TSubclassOf<UGameplayAbility>> InitialAbilities;
	
	//Attribute Sets
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AM|GAS")
	int32 StartLevel;
	
};
