// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Net_GameState.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRemainingTimeChanged, int32, NewTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerAdded, APlayerState*, NewPlayerState);

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
	
	virtual void AddPlayerState(APlayerState* PlayerState) override;
	
	UFUNCTION()
	void OnRep_RemainingTime();

public:
	UFUNCTION(NetMulticast, Reliable)
	void GameOverRPC(int32 WinnerPlayerIdx);

	void SetRemainingTime(int32 NewTime);
	
	FORCEINLINE int32 GetRemainingTime() const { return RemainingTime; }
	
protected:
	UPROPERTY(BlueprintReadOnly, Category = "AM|Variable", ReplicatedUsing=OnRep_RemainingTime)
	int32 RemainingTime;
	
public:
	UPROPERTY(BlueprintAssignable, Category= "AM|Events")
	FOnRemainingTimeChanged OnRemainingTimeChanged;
	
	UPROPERTY(BlueprintAssignable, Category= "AM|Events")
	FOnPlayerAdded OnPlayerAdded;
	
};
