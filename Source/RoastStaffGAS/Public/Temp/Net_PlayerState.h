// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/K_PlayerState.h"
#include "Net_PlayerState.generated.h"

class UK_NetAttributeSet;
/**
 * 
 */
UCLASS()
class ROASTSTAFFGAS_API ANet_PlayerState : public AK_PlayerState
{
	GENERATED_BODY()
	
public:
	ANet_PlayerState();
	
	UFUNCTION(BlueprintCallable, Category = "AM|Net")
	UK_NetAttributeSet* GetNetAttributeSet() const {return NetAttributeSet; }
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AM|Net")
	TObjectPtr<UK_NetAttributeSet> NetAttributeSet;
	
};
