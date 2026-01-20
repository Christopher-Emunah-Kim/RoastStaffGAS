// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Net_GameState.generated.h"

class ANet_PlayerState;
/**
 * 
 */
UCLASS()
class ROASTSTAFFGAS_API ANet_GameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	ANet_GameState();

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION()
	void OnRep_RemainingTime();

public:
	UFUNCTION(NetMulticast, Reliable)
	void GameOverRPC(int32 WinnerPlayerIdx);
	
	FORCEINLINE int32 GetRemainingTime() const { return RemainingTime; }
	
protected:
	UPROPERTY(BlueprintReadOnly, Category = "AM|Variable", ReplicatedUsing=OnRep_RemainingTime)
	int32 RemainingTime;
	
};
