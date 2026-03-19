// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/RSGameMode.h"

#include "RoastStaffGAS.h"
#include "Subsystems/PoolingSubsystem.h"

void ARSGameMode::BeginPlay()
{
	Super::BeginPlay();          
	
    InitializePools();     
}

void ARSGameMode::InitializePools()
{
	GET_WORLD_SUBSYSTEM(UPoolingSubsystem, PoolSys);                                                           
                                                                                                                   
	for (const FPoolInitEntry& Entry : PoolEntries)                                                            
	{                                                                                                          
		if (!Entry.ActorClass)                                                                             
		{                                                                                                  
			KHS_WARN(TEXT("PoolEntry에 ActorClass가 비어있음. 건너뜀."));
			continue;                                                                                  
		}                                                                                                  
		PoolSys->InitializePool(Entry.ActorClass, Entry.Count);                                            
	}   
}
