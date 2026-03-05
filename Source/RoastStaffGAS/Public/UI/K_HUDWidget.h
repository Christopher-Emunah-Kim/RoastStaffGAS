// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/K_BaseWidget.h"
#include "K_HUDWidget.generated.h"

class UAbilitySystemComponent;
class UK_StateBarWidget;
/**
 * 
 */
UCLASS()
class ROASTSTAFFGAS_API UK_HUDWidget : public UK_BaseWidget
{
	GENERATED_BODY()
	
	UK_HUDWidget();
	
public:
	void BindToASC(UAbilitySystemComponent* InASC);
	
// private:
// 	UPROPERTY(meta = (BindWidgetOptional))
// 	TObjectPtr<UK_StateBarWidget> WBP_StateBar;
};
