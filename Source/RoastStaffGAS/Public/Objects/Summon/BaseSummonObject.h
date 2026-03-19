// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/RuntimeDataStructs.h"
#include "GameFramework/Actor.h"
#include "Interface/PoolableInterface.h"
#include "BaseSummonObject.generated.h"

/**                                                                                                              
   * ABaseSummonObject                                                                                               
   * 소환형 스킬 장판 오브젝트.                                                                                    
   * SummonRadius 내 Enemy에 DamageGE + StatusGE 즉시 적용.                                    
   * Lifetime 후 자동 소멸.                                                                                        
   */   
class UNiagaraComponent;

UCLASS()
class ROASTSTAFFGAS_API ABaseSummonObject : public AActor, public IPoolableInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABaseSummonObject();
	
	void InitSummon(const FSummonObjectInitData& InInitData);         
	
	//IPoolableInterface
	void OnPoolActivate() override;  
	void OnPoolDeactivate() override;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	void ApplyGameplayEffectToArea(); 
	void OnLifetimeExpired();    
private:
	UPROPERTY()                                                                                                  
	FSummonObjectInitData InitData;                                                                              
	
	FTimerHandle LifetimeHandle; 
	bool bInitialized = false; 
	
protected:
	UPROPERTY(VisibleAnywhere, Category = "MY|VFX")                                                              
	TObjectPtr<UNiagaraComponent> VFXComp;  

};
