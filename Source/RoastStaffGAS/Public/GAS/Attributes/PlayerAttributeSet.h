// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Attributes/BaseAttributeSet.h"
#include "PlayerAttributeSet.generated.h"


// EXP 변경 감지 델리게이트 — 레벨업 시스템이 구독
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEXPChanged, float, NewEXP, int32, CurrentLevel);

/**
 * 플레이어 전용 AttributeSet
 * ATK, DEF, AttackSpeed, CastingSpeed, CriticalRate, CriticalDamage, EXP, Level
 */
UCLASS()
class ROASTSTAFFGAS_API UPlayerAttributeSet : public UBaseAttributeSet
{
	GENERATED_BODY()
	public:
    UPlayerAttributeSet();

    virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
    virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	
	// 델리게이트
	FOnEXPChanged OnEXPChangedDel;
	
	// -------------------------------------------------------------------------
	// 어트리뷰트
	// -------------------------------------------------------------------------
    UPROPERTY(BlueprintReadOnly, Category = "Vanguard|Attribute|Player")
    FGameplayAttributeData ATK;
    ATTRIBUTE_ACCESSORS(UPlayerAttributeSet, ATK)

    UPROPERTY(BlueprintReadOnly, Category = "Vanguard|Attribute|Player")
    FGameplayAttributeData DEF;
    ATTRIBUTE_ACCESSORS(UPlayerAttributeSet, DEF)

    UPROPERTY(BlueprintReadOnly, Category = "Vanguard|Attribute|Player")
    FGameplayAttributeData AttackSpeed;
    ATTRIBUTE_ACCESSORS(UPlayerAttributeSet, AttackSpeed)

    UPROPERTY(BlueprintReadOnly, Category = "Vanguard|Attribute|Player")
    FGameplayAttributeData CastingSpeed;
    ATTRIBUTE_ACCESSORS(UPlayerAttributeSet, CastingSpeed)

    /** 치명타 확률 (0.0 ~ 1.0) */
    UPROPERTY(BlueprintReadOnly, Category = "Vanguard|Attribute|Player")
    FGameplayAttributeData CriticalRate;
    ATTRIBUTE_ACCESSORS(UPlayerAttributeSet, CriticalRate)

    /** 치명타 배율 (기본 1.5) */
    UPROPERTY(BlueprintReadOnly, Category = "Vanguard|Attribute|Player")
    FGameplayAttributeData CriticalDamage;
    ATTRIBUTE_ACCESSORS(UPlayerAttributeSet, CriticalDamage)

    UPROPERTY(BlueprintReadOnly, Category = "Vanguard|Attribute|Player")
    FGameplayAttributeData EXP;
    ATTRIBUTE_ACCESSORS(UPlayerAttributeSet, EXP)

    /** GE를 통한 레벨 관리. 정수지만 GAS 호환을 위해 float 사용 */
    UPROPERTY(BlueprintReadOnly, Category = "Vanguard|Attribute|Player")
    FGameplayAttributeData Level;
    ATTRIBUTE_ACCESSORS(UPlayerAttributeSet, Level)


private:
    // EXP/Level 하한 보호용 내부 클램프
    void ClampPositive(const FGameplayAttribute& Attribute, float& NewValue) const;

};
