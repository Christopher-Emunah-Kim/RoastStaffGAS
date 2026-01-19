// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "K_NetAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

constexpr int32 ITEM_MAX_COUNT = 100;

struct FGameplayEffectModCallbackData;

/**
 * 
 */
UCLASS()
class ROASTSTAFFGAS_API UK_NetAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	UK_NetAttributeSet();
	
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;	
	
	UFUNCTION()
	void OnRep_ItemCount(const FGameplayAttributeData& OldValue);
	
public:
	UPROPERTY(BlueprintReadOnly, Category = "Network", ReplicatedUsing=OnRep_ItemCount)
	FGameplayAttributeData ItemCount;
	ATTRIBUTE_ACCESSORS(UK_NetAttributeSet, ItemCount);
	
};
