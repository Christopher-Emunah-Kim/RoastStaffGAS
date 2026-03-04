// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RoastStaffGAS.h"
#include "K_BaseAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

struct FGameplayEffectModCallbackData;

/**
 * 
 */
UCLASS()
class ROASTSTAFFGAS_API UK_BaseAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	UK_BaseAttributeSet();
	
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;	
	
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldValue);
	
	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);
	
	UFUNCTION()
	void OnRep_Mana(const FGameplayAttributeData& OldValue);
	
	UFUNCTION()
	void OnRep_MaxMana(const FGameplayAttributeData& OldValue);
	
	
private:
	//사망 이벤트 전송 용도
	void HandleDeath(const FGameplayEffectModCallbackData& Data);
	//데미지 수신 이벤트 전송 용도
	void BroadcastDamageEvent(const FGameplayEffectModCallbackData& Data, float Damage);

public:
	//기본 어트리뷰트
	UPROPERTY(BlueprintReadOnly, Category = "AM|Health", ReplicatedUsing=OnRep_Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UK_BaseAttributeSet, Health)
	
	UPROPERTY(BlueprintReadOnly, Category = "AM|Health", ReplicatedUsing = OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UK_BaseAttributeSet, MaxHealth)
	
	UPROPERTY(BlueprintReadOnly, Category = "AM|Mana", ReplicatedUsing = OnRep_Mana)
	FGameplayAttributeData Mana;
	ATTRIBUTE_ACCESSORS(UK_BaseAttributeSet, Mana)
	
	UPROPERTY(BlueprintReadOnly, Category = "AM|Mana", ReplicatedUsing = OnRep_MaxMana)
	FGameplayAttributeData MaxMana;
	ATTRIBUTE_ACCESSORS(UK_BaseAttributeSet, MaxMana)
	
	//메타 어트리뷰트
	UPROPERTY(BlueprintReadOnly, Category = "Damage")
	FGameplayAttributeData IncomingDamage;
	ATTRIBUTE_ACCESSORS(UK_BaseAttributeSet, IncomingDamage)
	
	
	
};
