// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "BaseAttributeSet.generated.h"

// GAS 어트리뷰트 접근자 매크로 (Get/Set/Init 자동 생성)
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

// 사망 감지 델리게이트 — BaseCharacter가 구독
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeath);

// HP 변경 감지 델리게이트 — UI가 구독
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, float, NewValue, float, MaxValue);

/**
 * 플레이어/에너미 공통 AttributeSet
 * MaxHP, CurrentHP, MoveSpeed 관리
 */

UCLASS()
class ROASTSTAFFGAS_API UBaseAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	UBaseAttributeSet();

	// UAttributeSet 오버라이드
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	// 델리게이트
	FOnDeath OnDeathDel;
	FOnHealthChanged OnHealthChangedDel;
	
	// 어트리뷰트
	UPROPERTY(BlueprintReadOnly, Category = "MY|Attribute")
	FGameplayAttributeData MaxHP;
	ATTRIBUTE_ACCESSORS(UBaseAttributeSet, MaxHP)

	UPROPERTY(BlueprintReadOnly, Category = "MY|Attribute")
	FGameplayAttributeData CurrentHP;
	ATTRIBUTE_ACCESSORS(UBaseAttributeSet, CurrentHP)

	UPROPERTY(BlueprintReadOnly, Category = "MY|Attribute")
	FGameplayAttributeData MoveSpeed;
	ATTRIBUTE_ACCESSORS(UBaseAttributeSet, MoveSpeed)
	
};
