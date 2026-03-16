// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/RSBaseWidget.h"
#include "RSHUDWidget.generated.h"


class UWeaponSlotContainerWidget;

/**
 * 
 */
UCLASS()
class ROASTSTAFFGAS_API URSHUDWidget : public URSBaseWidget
{
	GENERATED_BODY()
	
	URSHUDWidget();
	
protected:
	virtual void NativeConstruct() override;
	
public:
	FORCEINLINE UWeaponSlotContainerWidget* GetSlotContainerWidget() const  { return WBP_SlotContainer; }
	
private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWeaponSlotContainerWidget> WBP_SlotContainer;

	
};
