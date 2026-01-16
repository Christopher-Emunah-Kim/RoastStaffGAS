// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EnemyStateWidget.generated.h"


class UProgressBar;
class UTextBlock;
class UAbilitySystemComponent;
struct FOnAttributeChangeData;
/*
* UK_EnemyHealthWidget
 * 
 * 적의 Health를 표시하는 UI 위젯
 * 적 머리 위에 WidgetComponent로 배치
 */
UCLASS()
class ROASTSTAFFGAS_API UEnemyStateWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	void OnHealthChanged(const FOnAttributeChangeData& Data);
	void OnMaxHealthChanged(const FOnAttributeChangeData& Data);
	void SetHealthValues(float CurrentHealth, float MaxHealth);
	
public:
	/**
	 * ASC를 바인딩하여 State 변경을 감지
	 */
	UFUNCTION(BlueprintCallable, Category = "AM|UI")
	void BindToASC(UAbilitySystemComponent* InASC);
    
	UFUNCTION(BlueprintCallable, Category = "AM|UI")
	void UpdateStateDisplay();
	
	
protected:
	/** 체력 게이지 바 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> bar_Health;
    
	/** 체력 텍스트  */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> txt_Health;

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> BoundASC;
    
	float CachedHealth = 0.f;
	float CachedMaxHealth = 0.f;
	
};
