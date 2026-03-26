// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/RSBaseWidget.h"
#include "WeaponSlotContainerWidget.generated.h"

/**
 * WeaponSlotWidget 컨테이너 HUD UI
 */

class UWeaponSlotWidget;

UCLASS()
class ROASTSTAFFGAS_API UWeaponSlotContainerWidget : public URSBaseWidget
{
	GENERATED_BODY()

public:
	UWeaponSlotWidget* GetSlotWidget(int32 SlotIndex) const;

protected:
	virtual void NativeConstruct() override;

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWeaponSlotWidget> SlotWidget_0;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWeaponSlotWidget> SlotWidget_1;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWeaponSlotWidget> SlotWidget_2;

};
