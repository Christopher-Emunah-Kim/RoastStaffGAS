// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "PoolingSubsystem.generated.h"

/**                                                                                                              
 * URSPoolingSubsystem (UWorldSubsystem)                                                                         
 * 인게임 오브젝트의 Spawn/Destroy GC 부하를 방지하는 풀링 서브시스템.                                         
 *                                                                                                               
 * Actor Pool : AActor 파생 전체 (SummonObject, Projectile, Enemy 등)
 * Widget Pool: 이후 스프린트 확장용 — 현재 스켈레톤만 선언                                                      
 *                                                                                                               
 * 접근: GetWorld()->GetSubsystem<URSPoolingSubsystem>()                                                         
 */  
UCLASS()
class ROASTSTAFFGAS_API UPoolingSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
  	/** 스테이지 시작 시 풀링 */                                          
  	void InitializePool(TSubclassOf<AActor> ActorClass, int32 Count);
	/** 풀에서 꺼내거나 신규 스폰. */                                                     
  	AActor* SpawnPooledActor(TSubclassOf<AActor> ActorClass, const FTransform& SpawnTransform);                  
  	/** 풀 반납하고 비활성화 */                                                            
  	void ReturnToPool(AActor* Actor);                                                                            
  	/** 전체 반납 */                                        
  	void ReturnAllActiveActors();                                                                                
  	                                                                                                             
  	/** 타입 안전 헬퍼 — Cast 생략용 */                                                                          
  	template<typename T>
  	T* SpawnPooledActor(TSubclassOf<T> ActorClass, const FTransform& SpawnTransform)                             
  	{                                                                                                            
  	    return Cast<T>(SpawnPooledActor(TSubclassOf<AActor>(ActorClass), SpawnTransform));
  	}                                                                                                            
                                      
private:
	bool TrySpawnActor(TSubclassOf<AActor> ActorClass, AActor*& Actor);
	
private:                                                                                                         
  	// ── Actor Pool ────────────────────────────────────────────────────────────                              
  	/** 비활성 액터 풀 */
  	TMap<UClass*, TArray<TObjectPtr<AActor>>> ActorPool;                                                        
  	/** 활성 액터 추적 */                                  
  	UPROPERTY() //GC방지 강한 참조 필수                                                                                              
  	TSet<TObjectPtr<AActor>> ActiveActors;                                                                       
  	                                                                                                             
  	// ── Widget Pool (데미지 플로팅 위젯 등) ─────────                                
  	// TMap<UClass*, TArray<TObjectPtr<UUserWidget>>> WidgetPool;                                                
  	// UPROPERTY()                                                                                               
  	// TSet<TObjectPtr<UUserWidget>> ActiveWidgets; 
	
};
