// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "K_GameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class ROASTSTAFFGAS_API AK_GameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
	virtual void BeginPlay() override;
	
	void TestGasModules();
};
