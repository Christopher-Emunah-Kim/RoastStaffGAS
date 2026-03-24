// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystems/PoolingSubsystem.h"
#include "RoastStaffGAS.h"
#include "Interface/PoolableInterface.h"

void UPoolingSubsystem::InitializePool(TSubclassOf<AActor> ActorClass, int32 Count)
{
	if (!ActorClass || Count <= 0)                                                                                             
    {                                                                                                          
        KHS_WARN(TEXT("ActorClass is null or Invalid Count : %d"), Count);
        return;
    }
    
                                                                                                                 
    TArray<TObjectPtr<AActor>>& Pool = ActorPool.FindOrAdd(ActorClass);                                          
                                                                                                                 
    for (int32 i = 0; i < Count; ++i)                                                                            
    {         
        AActor* Actor = nullptr;
        if (!TrySpawnActor(ActorClass, Actor))
        {
            KHS_WARN(TEXT("SpawnActor 실패 — Class: %s [%d/%d]"), *ActorClass->GetName(), i + 1, Count);                                                           
            continue;       
        }
        
        // Actor::BeginPlay → OnPoolDeactivate 호출 → 비활성 상태로 시작                                         
        Pool.Add(Actor);                                                                                         
    }
                                                                                                                 
    KHS_INFO(TEXT("InitializePool 완료 — Class: %s, Count: %d"), *ActorClass->GetName(), Pool.Num());
}

bool UPoolingSubsystem::TrySpawnActor(TSubclassOf<AActor> ActorClass, AActor*& Actor)
{
    FActorSpawnParameters Params;                                                                            
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;               
    Actor = GetWorld()->SpawnActor<AActor>(ActorClass, FTransform::Identity, Params);                        
                                                                                                                   
    if (!Actor)                                                                                              
    {
        return false;
    }
    return true;
}

AActor* UPoolingSubsystem::SpawnPooledActor(TSubclassOf<AActor> ActorClass, const FTransform& SpawnTransform)
{
    if (!ActorClass)                                                                                             
    {                                                                                                          
        KHS_WARN(TEXT("ActorClass null"));
        return nullptr;
    }
                                                                                                                 
    AActor* Actor = nullptr;
                                                                                                                 
    // 풀에서 유효 액터 팝                                                                                       
    if (TArray<TObjectPtr<AActor>>* Pool = ActorPool.Find(ActorClass))
    {                                                                                                            
        while (!Pool->IsEmpty())                                                                                 
        {                                                                                                        
            TObjectPtr<AActor> Candidate = Pool->Pop();                                                          
            if (IsValid(Candidate))                                                                              
            {                                                                                                    
                Actor = Candidate;                                                                               
                break;                                                                                           
            }                                                                                                    
        }                                                                                                      
    }

    // 풀에 없다면 신규 스폰 시도                                       
    if (!Actor)
    {                                                                                                            
        KHS_WARN(TEXT("풀 비어있음 → 신규 스폰. Class: %s"), *ActorClass->GetName());                                                                             
   
        if (!TrySpawnActor(ActorClass, Actor))
        {
            KHS_WARN(TEXT("SpawnPooledActor: SpawnActor 실패. Class: %s"), *ActorClass->GetName());  
            return nullptr;
        }
    }                                                                                                          

    Actor->SetActorTransform(SpawnTransform);

    if (IPoolableInterface* Interface = Cast<IPoolableInterface>(Actor))                                     
    {
        Interface->OnPoolActivate();                                                                             
    }                                                                                                          
                                                                                                                 
    ActiveActors.Add(Actor);                                                                                   
    return Actor;

}

void UPoolingSubsystem::ReturnToPool(AActor* Actor)
{
    if (!IsValid(Actor))                                                                                         
    {                                                                                                          
        KHS_WARN(TEXT("유효하지 않은 Actor"));
        return;                                                                                                  
    }
                                                                                                                   
    if (!ActiveActors.Contains(Actor))                                                                         
    {
        KHS_WARN(TEXT("이중 반납 방지. Actor: %s"), *Actor->GetName());
        return;                                                                                                  
    }
                                                                                                                   
    ActiveActors.Remove(Actor);                                                                                

    if (IPoolableInterface* Interface = Cast<IPoolableInterface>(Actor))                                     
    {
        Interface->OnPoolDeactivate();                                                                           
    }                                                                                                          
                                                                                                                   
    ActorPool.FindOrAdd(Actor->GetClass()).Add(Actor);   
}

void UPoolingSubsystem::ReturnAllActiveActors()
{
    //조회 중 수정되는 상황 방지하기 위해 배열 따로 만들어 처리
    TArray<TObjectPtr<AActor>> Snapshot(ActiveActors.Array());                                                   
    for (TObjectPtr<AActor>& Actor : Snapshot)                                                                   
    {                                                                                                            
        if (IsValid(Actor))                                                                                      
        {                                                                                                        
            ReturnToPool(Actor);
        }                                                                                                        
    }     
}
