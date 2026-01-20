// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "Blueprint/UserWidget.h"
#include "Net_HUD.generated.h"

struct FOnAttributeChangeData;
class ANet_GameState;
class UTextBlock;

/**
 * 
 */
UCLASS()
class ROASTSTAFFGAS_API UNet_HUD : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
	UFUNCTION()
	void OnTimeChanged(int32 NewTime);
	
	void OnPlayerScoreChanged(const FOnAttributeChangeData& Data);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
	void ShowGameResult(int32 WinnerIndex);
	
private:
	void BindToGameState();
	void UnbindFromGameState();
	void UpdatePlayerScore();
	
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> txt_Timer;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> txt_Player1Score;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> txt_Player2Score;
	
	UPROPERTY()
	TObjectPtr<ANet_GameState> NetGameState;
	
	int32 LastRemainingTime= 60;
	
	
};
