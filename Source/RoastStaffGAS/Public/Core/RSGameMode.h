// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "RSGameMode.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)                                                                                           
struct FPoolInitEntry                                                                                            
{                                                                                                                
	GENERATED_BODY()                                                                                         
                                                                                                                   
	UPROPERTY(EditDefaultsOnly)                                                                                
	TSubclassOf<AActor> ActorClass;                                                                            
                                                                                                                   
	UPROPERTY(EditDefaultsOnly)                                                                                
	int32 Count = 20;                                                                                          
};       

UCLASS()
class ROASTSTAFFGAS_API ARSGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
protected:                                                                                                       
	virtual void BeginPlay() override;                                                                         
                                                                                                                   
private:                                                                                                         
	void InitializePools();                                                                                    
                                                                                                                   
	UPROPERTY(EditDefaultsOnly, Category = "Pooling")                                                        
	TArray<FPoolInitEntry> PoolEntries;   
	
};
