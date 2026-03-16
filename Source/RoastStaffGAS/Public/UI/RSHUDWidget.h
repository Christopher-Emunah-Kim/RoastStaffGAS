// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/RSBaseWidget.h"
#include "RSHUDWidget.generated.h"


//class URSSlotContainerWidget;

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
	
	//FORCEINLINE URSSlotContainerWidget* GetSlotContainerWidget() const  { return WBP_SlotContainer; }
	
private:
	//UPROPERTY(meta = (BindWidget))
	//TObjectPtr<URSSlotContainerWidget> WBP_SlotContainer;

	
};
