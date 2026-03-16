// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "RSPlayerController.generated.h"

/**
 * 
 */

class URSHUDWidget;

UCLASS()
class ROASTSTAFFGAS_API ARSPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UFUNCTION()
	void OnSlotUpdated(int32 SlotIndex);
	void RefreshSlotUI(int32 SlotIndex);

protected:
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<URSHUDWidget> HUDWidgetClass;
	UPROPERTY()
	TObjectPtr<URSHUDWidget> CachedHUDUI;
};
