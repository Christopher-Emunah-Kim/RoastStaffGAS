// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "K_BaseCharacter.generated.h"

class UGameplayAbility;
class UAbilitySystemComponent;
class UK_BaseAttributeSet;

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

	//GAS 초기화
	virtual void InitializeAbilitySystem();
	
public:
	//IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	//AttributeSet Getter
	UFUNCTION(BlueprintCallable, Category = "AM|GAS")
	UK_BaseAttributeSet* GetAttributeSet() const {return BaseAttributeSet;}
	
	UFUNCTION(BlueprintCallable, Category = "AM|GAS|Attributes")
	float GetHealth() const;
	
	UFUNCTION(BlueprintCallable, Category = "AM|GAS|Attributes")
	float GetMaxHealth() const;
	
	UFUNCTION(BlueprintCallable, Category = "AM|GAS|Attributes")
	float GetMana() const;
	
	UFUNCTION(BlueprintCallable, Category = "AM|GAS|Attributes")
	float GetMaxMana() const;
	
	UFUNCTION(BlueprintImplementableEvent)
	void MyPostInitailizeComponents();

private:
	bool bASCInitialized;
	
protected:
	/* //////////////////////
	 * GAS Contents
	 *///////////////////////
	
	//ASC
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AM|GAS")
	TObjectPtr<class UAbilitySystemComponent> ASC;
	
	//Attribute Sets
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AM|GAS")
	TObjectPtr<UK_BaseAttributeSet> BaseAttributeSet;
	
	//Game Abilities
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AM|GAS")
	TArray<TSubclassOf<UGameplayAbility>> InitialAbilities;
	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AM|GAS")
	int32 CharacterLevel;
	
};
