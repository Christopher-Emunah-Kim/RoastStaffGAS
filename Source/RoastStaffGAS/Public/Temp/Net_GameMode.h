// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Net_GameMode.generated.h"

class ANet_GameState;
/**
 * 
 */
UCLASS()
class ROASTSTAFFGAS_API ANet_GameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	ANet_GameMode();
	
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	
	int32 DetermineWinner();
	
	void EndGame();
	
protected:
	UPROPERTY()
	TObjectPtr<ANet_GameState> NetGameState;
	
	bool bGameEnded;
	
};
