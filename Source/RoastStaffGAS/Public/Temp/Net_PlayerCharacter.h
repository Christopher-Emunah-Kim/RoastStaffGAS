// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/K_PlayerCharacter.h"
#include "Net_PlayerCharacter.generated.h"

class UK_NetAttributeSet;
class UGameplayEffect;
/**
 * 
 */
UCLASS()
class ROASTSTAFFGAS_API ANet_PlayerCharacter : public AK_PlayerCharacter
{
	GENERATED_BODY()
	
public:
	ANet_PlayerCharacter();
	
protected:
	virtual void PossessedBy(AController* NewController) override;
	virtual void InitializeAbilitySystem() override;
	virtual void OnRep_PlayerState() override;
	
public:
	UK_NetAttributeSet* GetNetAttributeSet() const;
	
	UFUNCTION(BlueprintNativeEvent, Category = "AM|Pickup")
	void AddPickUp();
	virtual void AddPickUp_Implementation();
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Network|Pickup")
	TSubclassOf<UGameplayEffect> PickUpEffect;
	
};
