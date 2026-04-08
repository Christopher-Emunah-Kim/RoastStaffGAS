// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Subsystems/WorldSubsystem.h"
#include "PoolingSubsystem.generated.h"

/**
 * FPoolPreWarmRequest
 * PoolingSubsystem::RequestAsyncPreWarm()에 전달하는 프리웜 요청 단위.
 * ActorClass 또는 WidgetClass 중 하나만 유효하면 됨 (나머지는 nullptr).
 */
USTRUCT(BlueprintType)
struct FPoolPreWarmRequest
{
	GENERATED_BODY()

	/** 프리웜할 Actor 클래스. Widget 요청이면 nullptr. */
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AActor> ActorClass = nullptr;
	/** 프리웜할 Widget 클래스. Actor 요청이면 nullptr. */
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> WidgetClass = nullptr;

	/** 프리웜할 인스턴스 수 */
	UPROPERTY(EditDefaultsOnly)
	int32 Count = 0;
};

/**
 * FActorPoolBucket
 * UHT는 TMap<Key, TArray<Value>> 중첩 컨테이너에 UPROPERTY를 지원하지 않음.
 * TArray를 USTRUCT로 래핑해 GC 추적을 확보
 */
USTRUCT()
struct FActorPoolBucket
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<TObjectPtr<AActor>> Actors;
};

/** FWidgetPoolBucket — FActorPoolBucket 동일 패턴, Widget용 */
USTRUCT()
struct FWidgetPoolBucket
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<TObjectPtr<UUserWidget>> Widgets;
};

/**
 * UPoolingSubsystem (UTickableWorldSubsystem)
 * 인게임 오브젝트의 Spawn/Destroy GC 부하를 방지하는 풀링 서브시스템.
 *
 * Actor Pool : AActor 파생 전체 (SummonObject, Projectile, Enemy 등)
 * Widget Pool: UUserWidget 파생 (FloatingDamage 등)
 *
 * 접근: GetWorld()->GetSubsystem<UPoolingSubsystem>()
 */
UCLASS()
class ROASTSTAFFGAS_API UPoolingSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	// ── UTickableWorldSubsystem ───────────────────────────────────────────────
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override;
	
	// ── Actor Pool ───────────────────────────────────────────────────────────
	/** 스테이지 시작 시 즉시 풀 채우기 */
	void InitializePool(TSubclassOf<AActor> ActorClass, int32 Count);
	/** 비동기 프리웜 요청 — Tick에서 프레임당 PreWarmBatchSize개씩 스폰 */
	void RequestAsyncPreWarm(TArray<FPoolPreWarmRequest> Requests);
	/** 프리웜 진행률 0.0~1.0. 요청 없으면 1.0 반환 */
	float GetPreWarmProgress() const;

	/** 풀에서 꺼내거나 신규 스폰 */
	AActor* SpawnPooledActor(TSubclassOf<AActor> ActorClass, const FTransform& SpawnTransform);
	/** 풀 반납하고 비활성화 */
	void ReturnToPool(AActor* Actor);
	/** 전체 반납 */
	void ReturnAllActiveActors();
	/** 특정 클래스 활성 액터 강제 반납 후 풀 버킷 제거 — 무기 교체 시 사용 */
	void DrainPool(TSubclassOf<AActor> ActorClass);

	// ── Widget Pool ───────────────────────────────────────────────────────────
	/** 스테이지 시작 시 즉시 위젯 풀 채우기 */
	void InitializeWidgetPool(TSubclassOf<UUserWidget> WidgetClass, int32 Count, APlayerController* PC);
	/** 풀에서 꺼내거나 신규 생성. 뷰포트/가시성은 호출자가 처리 */
	UUserWidget* SpawnPooledWidget(TSubclassOf<UUserWidget> WidgetClass, APlayerController* PC);
	/** 풀 반납 + Collapsed 처리 */
	void ReturnWidgetToPool(UUserWidget* Widget);

	/** 타입 안전 헬퍼 — Cast 생략용 */
	template<typename T>
	T* SpawnPooledActor(TSubclassOf<T> ActorClass, const FTransform& SpawnTransform)
	{
		return Cast<T>(SpawnPooledActor(TSubclassOf<AActor>(ActorClass), SpawnTransform));
	}

private:
	bool TrySpawnActor(TSubclassOf<AActor> ActorClass, AActor*& Actor);
	/** Actor 스폰 후 비활성 버킷에 적재 — InitializePool / TickPreWarm 공용 */
	bool AddActorToPool(TSubclassOf<AActor> ActorClass);
	/** PreWarm 요청 1단위 처리. false = Widget + PC 미준비(루프 중단) */
	bool SpawnOnePreWarmUnit(FPoolPreWarmRequest& Req);
	/** 프레임당 PreWarmBatchSize개 배치 스폰 */
	void TickPreWarm();
	
	/** 풀 배열에서 유효 인스턴스 팝 탬플릿 — GC 무효화된 항목 자동 건너뜀 */
	template<typename T>
	T* PopFirstValid(TArray<TObjectPtr<T>>& Pool);


public:
	/** 프리웜 전체 완료 시 1회 브로드캐스트 */
	FSimpleMulticastDelegate OnPreWarmComplete;
	
private:
	// ── Actor Pool ────────────────────────────────────────────────────────────
	/** 비활성 액터 풀 — FActorPoolBucket 래퍼로 UHT 중첩 컨테이너 제한 우회 */
	UPROPERTY()
	TMap<TObjectPtr<UClass>, FActorPoolBucket> ActorPool;
	/** 활성 액터 추적 — GC 방지 강한 참조 필수 */
	UPROPERTY()
	TSet<TObjectPtr<AActor>> ActiveActors;

	// ── PreWarm ───────────────────────────────────────────────────────────────
	/** 프레임당 배치 스폰 수 */
	UPROPERTY(EditDefaultsOnly, Category = "PreWarm")
	int32 PreWarmBatchSize = 5;

	/** 대기 중인 프리웜 요청 큐 — UPROPERTY로 TSubclassOf GC 추적 */
	UPROPERTY()
	TArray<FPoolPreWarmRequest> PreWarmQueue;

	int32 PreWarmTotalCount = 0;
	int32 PreWarmDoneCount = 0;

	// ── Widget Pool ───────────────────────────────────────────────────────────
	/** 비활성 위젯 풀 — FWidgetPoolBucket 래퍼로 UHT 중첩 컨테이너 제한 우회 */
	UPROPERTY()
	TMap<TObjectPtr<UClass>, FWidgetPoolBucket> WidgetPool;
	/** 활성 위젯 추적 — GC 방지 강한 참조 필수 */
	UPROPERTY()
	TSet<TObjectPtr<UUserWidget>> ActiveWidgets;
};



//=================================================================
// 탬플릿 구현부
//=================================================================

template<typename T>
T* UPoolingSubsystem::PopFirstValid(TArray<TObjectPtr<T>>& Pool)
{
	while (!Pool.IsEmpty())
	{
		TObjectPtr<T> Candidate = Pool.Pop();
		if (IsValid(Candidate))
		{
			return Candidate.Get();
		}
	}
	return nullptr;
}
