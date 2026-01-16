// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "K_StateBarWidget.generated.h"

class UProgressBar;
class UTextBlock;
class UAbilitySystemComponent;
struct FOnAttributeChangeData;

/**
 * UK_StateBarWidget
 * 
 * 플레이어의 Health/Mana를 표시하는 UI 위젯입니다.
 * 화면 좌하단에 배치됩니다.
 * 
 */
UCLASS()
class ROASTSTAFFGAS_API UK_StateBarWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	void OnHealthChanged(const FOnAttributeChangeData& Data);
	void OnMaxHealthChanged(const FOnAttributeChangeData& Data);
	void OnManaChanged(const FOnAttributeChangeData& Data);
	void OnMaxManaChanged(const FOnAttributeChangeData& Data);
	
	void SetHealthValues(float CurrentHealth, float MaxHealth);
	void SetManaValues(float CurrentMana, float MaxMana);
	
public:
	/**
	 * ASC를 바인딩하여 Health/Mana 변경을 감지
	 * 위젯 생성 후 반드시 호출
	 * @param InASC 플레이어의 ASCcomp
	 */
	UFUNCTION(BlueprintCallable, Category = "AM|UI")
	void BindToASC(UAbilitySystemComponent* InASC);
    
	/**
	 * UI를 수동으로 업데이트
	 * 바인딩 직후 초기값을 표시할 때 사용
	 */
	UFUNCTION(BlueprintCallable, Category = "AM|UI")
	void UpdateStateBarDisplay();
	
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> bar_Health;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> txt_Health;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> bar_Mana;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> txt_Mana;
	
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> BoundASC;
	
	
	float CachedHealth = 0.f;
	float CachedMaxHealth = 0.f;
	float CachedMana = 0.f;
	float CachedMaxMana = 0.f;
	
};
