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

    for (int32 i = 0; i < Count; ++i)
    {
        if (!AddActorToPool(ActorClass))
        {
            KHS_WARN(TEXT("SpawnActor 실패 — Class: %s [%d/%d]"), *ActorClass->GetName(), i + 1, Count);
        }
    }

    KHS_INFO(TEXT("InitializePool 완료 — Class: %s, Count: %d"),
        *ActorClass->GetName(), ActorPool.FindOrAdd(ActorClass).Actors.Num());
}

bool UPoolingSubsystem::TrySpawnActor(TSubclassOf<AActor> ActorClass, AActor*& Actor)
{
    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    Actor = GetWorld()->SpawnActor<AActor>(ActorClass, FTransform::Identity, Params);
    return Actor != nullptr;
}

bool UPoolingSubsystem::AddActorToPool(TSubclassOf<AActor> ActorClass)
{
    AActor* Actor = nullptr;
    if (!TrySpawnActor(ActorClass, Actor))
    {
        return false;
    }
    // BeginPlay → OnPoolDeactivate 호출 → 비활성 상태로 버킷에 적재
    ActorPool.FindOrAdd(ActorClass).Actors.Add(Actor);
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

    if (FActorPoolBucket* Bucket = ActorPool.Find(ActorClass))
    {
        Actor = PopFirstValid(Bucket->Actors);
    }

    if (!Actor)
    {
        KHS_WARN(TEXT("풀 비어있음 → 신규 스폰. Class: %s"), *ActorClass->GetName());
        if (!TrySpawnActor(ActorClass, Actor))
        {
            KHS_WARN(TEXT("SpawnPooledActor: SpawnActor 실패. Class: %s"), *ActorClass->GetName());
            return nullptr;
        }
    }

    // Scale은 BP 기본값 유지 — 위치/회전만 적용
    Actor->SetActorLocationAndRotation(SpawnTransform.GetLocation(), SpawnTransform.GetRotation());

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
                                                                                                                   
    ActorPool.FindOrAdd(Actor->GetClass()).Actors.Add(Actor);   
}

void UPoolingSubsystem::ReturnAllActiveActors()
{
    // 조회 중 수정되는 상황 방지하기 위해 배열 따로 만들어 처리
    TArray<TObjectPtr<AActor>> Snapshot(ActiveActors.Array());
    for (TObjectPtr<AActor>& Actor : Snapshot)
    {
        if (IsValid(Actor))
        {
            ReturnToPool(Actor);
        }
    }
}

void UPoolingSubsystem::DrainPool(TSubclassOf<AActor> ActorClass)
{
    if (!ActorClass)
    {
        return;
    }

    // 해당 클래스 활성 액터 강제 반납 (in-flight 포함)
    TArray<TObjectPtr<AActor>> Snapshot(ActiveActors.Array());
    for (TObjectPtr<AActor>& Actor : Snapshot)
    {
        if (IsValid(Actor) && Actor->GetClass() == ActorClass)
        {
            ReturnToPool(Actor);
        }
    }

    // 풀 버킷 제거 — GC가 실제 해제 처리
    ActorPool.Remove(ActorClass);
    KHS_INFO(TEXT("DrainPool 완료 — Class: %s"), *ActorClass->GetName());
}

void UPoolingSubsystem::RequestAsyncPreWarm(TArray<FPoolPreWarmRequest> Requests)
{
    PreWarmQueue.Append(MoveTemp(Requests));

    // 총 스폰 수 재집계 (Append 이후 큐 전체 기준)
    PreWarmTotalCount = 0;
    PreWarmDoneCount = 0;
    for (const FPoolPreWarmRequest& Req : PreWarmQueue)
    {
        PreWarmTotalCount += FMath::Max(0, Req.Count);
    }

    KHS_INFO(TEXT("RequestAsyncPreWarm — 요청 %d건, 총 인스턴스 %d개"),
        PreWarmQueue.Num(), PreWarmTotalCount);
}

bool UPoolingSubsystem::IsTickable() const
{
    return !PreWarmQueue.IsEmpty();
}

TStatId UPoolingSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UPoolingSubsystem, STATGROUP_Tickables)
}

float UPoolingSubsystem::GetPreWarmProgress() const
{
    if (PreWarmTotalCount <= 0)
    {
        return 1.0f;
    }
    return FMath::Clamp(static_cast<float>(PreWarmDoneCount) / static_cast<float>(PreWarmTotalCount), 0.0f, 1.0f);
}

void UPoolingSubsystem::Tick(float DeltaTime)
{
    TickPreWarm();

    if (PreWarmQueue.IsEmpty() && PreWarmTotalCount > 0 && PreWarmDoneCount >= PreWarmTotalCount)
    {
        KHS_INFO(TEXT("프리웜 완료 — Done: %d / Total: %d"),
            PreWarmDoneCount, PreWarmTotalCount);

        PreWarmTotalCount = 0;
        PreWarmDoneCount = 0;

        OnPreWarmComplete.Broadcast();
    }
}

bool UPoolingSubsystem::SpawnOnePreWarmUnit(FPoolPreWarmRequest& Req)
{
    if (Req.ActorClass)
    {
        if (!AddActorToPool(Req.ActorClass))
        {
            KHS_WARN(TEXT("PreWarm Actor 스폰 실패 — Class: %s"), *Req.ActorClass->GetName());
        }
        return true;
    }

    if (Req.WidgetClass)
    {
        APlayerController* PC = GetWorld()->GetFirstPlayerController();
        if (!PC)
        {
            return false;  // PC 미준비 — 루프 중단, 다음 프레임 재시도
        }

        UUserWidget* Widget = CreateWidget<UUserWidget>(PC, Req.WidgetClass);
        if (!Widget)
        {
            KHS_WARN(TEXT("PreWarm Widget 생성 실패 — Class: %s"), *Req.WidgetClass->GetName());
        }
        else
        {
            Widget->SetVisibility(ESlateVisibility::Collapsed);
            WidgetPool.FindOrAdd(Req.WidgetClass).Widgets.Add(Widget);
        }
        return true;
    }

    return true;  // 유효하지 않은 요청 단위 — 스킵
}

void UPoolingSubsystem::TickPreWarm()
{
    int32 Budget = PreWarmBatchSize;
    int32 SpawnedThisFrame = 0;

    while (Budget > 0 && !PreWarmQueue.IsEmpty())
    {
        FPoolPreWarmRequest& Back = PreWarmQueue.Last();

        if (Back.Count <= 0)
        {
            PreWarmQueue.Pop();
            continue;
        }

        if (!SpawnOnePreWarmUnit(Back))
        {
            break;  // false = Widget + PC 미준비, 다음 프레임까지 대기
        }

        --Back.Count;
        --Budget;
        ++PreWarmDoneCount;
        ++SpawnedThisFrame;
    }

    if (SpawnedThisFrame > 0)
    {
        KHS_INFO(TEXT("TickPreWarm — 이번 프레임 %d개 스폰 | 진행: %d/%d"),
            SpawnedThisFrame, PreWarmDoneCount, PreWarmTotalCount);
    }
}

void UPoolingSubsystem::InitializeWidgetPool(TSubclassOf<UUserWidget> WidgetClass, int32 Count, APlayerController* PC)
{
    if (!WidgetClass || Count <= 0 || !PC)
    {
        KHS_WARN(TEXT("잘못된 인자 (WidgetClass/PC/Count 확인)"));
        return;
    }

    TArray<TObjectPtr<UUserWidget>>& Pool = WidgetPool.FindOrAdd(WidgetClass).Widgets;

    for (int32 i = 0; i < Count; ++i)
    {
        UUserWidget* Widget = CreateWidget<UUserWidget>(PC, WidgetClass);
        if (!Widget)
        {
            KHS_WARN(TEXT("CreateWidget 실패 [%d/%d]"), i + 1, Count);
            continue;
        }
        Widget->SetVisibility(ESlateVisibility::Collapsed);
        Pool.Add(Widget);
    }

    KHS_INFO(TEXT("InitializeWidgetPool 완료 — Class: %s, Count: %d"), *WidgetClass->GetName(), Pool.Num());
}

UUserWidget* UPoolingSubsystem::SpawnPooledWidget(TSubclassOf<UUserWidget> WidgetClass, APlayerController* PC)
{
    if (!WidgetClass || !PC)
    {
        KHS_WARN(TEXT("WidgetClass 또는 PC가 nullptr"));
        return nullptr;
    }

    UUserWidget* Widget = nullptr;

    if (FWidgetPoolBucket* Bucket = WidgetPool.Find(WidgetClass))
    {
        Widget = PopFirstValid(Bucket->Widgets);
    }

    if (!Widget)
    {
        KHS_WARN(TEXT("WidgetPool 비어있음 → 신규 생성. Class: %s"), *WidgetClass->GetName());
        Widget = CreateWidget<UUserWidget>(PC, WidgetClass);
        if (!Widget)
        {
            KHS_WARN(TEXT("CreateWidget 실패"));
            return nullptr;
        }
    }

    ActiveWidgets.Add(Widget);
    return Widget;
}

void UPoolingSubsystem::ReturnWidgetToPool(UUserWidget* Widget)
{
    if (!IsValid(Widget))
    {
        KHS_WARN(TEXT("유효하지 않은 Widget"));
        return;
    }

    if (!ActiveWidgets.Contains(Widget))
    {
        KHS_WARN(TEXT("이중 반납 방지. Widget: %s"), *Widget->GetName());
        return;
    }

    ActiveWidgets.Remove(Widget);
    Widget->SetVisibility(ESlateVisibility::Collapsed);
    WidgetPool.FindOrAdd(Widget->GetClass()).Widgets.Add(Widget);
}
