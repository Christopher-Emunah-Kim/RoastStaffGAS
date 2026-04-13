// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Data/RuntimeDataStructs.h"
#include "Subsystems/PoolingSubsystem.h"
#include "RSGameMode.generated.h"

class AEnemySpawner;
class URSLoadingWidget;
class ASummonPreviewObject;

/**
 * ARSGameMode
 *
 * - 스테이지 시작 진입점. 초기화 순서를 보장하는 조율자 역할만 담당한다.
 * - PreWarm: BeginPlay → PoolingSubsystem::RequestAsyncPreWarm → OnPreWarmCompleted → StartStageFlow
 * - 웨이브/스폰 로직: UStageManagerSubsystem에 위임.
 * - 스테이지 클리어/실패 판정 및 OUTGAME 복귀 처리
 */
UCLASS()
class ROASTSTAFFGAS_API ARSGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ARSGameMode();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:
	/** 플레이어 사망 시 외부(RSPlayerCharacter)에서 호출 */
	void OnStageFailed();
	
	FORCEINLINE TSubclassOf<ASummonPreviewObject> GetPreviewActorClass() const { return PreviewActorClass; }
	
private:
	// ── 초기화 흐름 ──────────────────────────────────────────────────────────
	void InitializePlayer(FName CharID);
	bool InitializeStage();
	/** PoolingSubsystem 프리웜 시작 + OnPreWarmComplete 구독 */
	void InitializePreWarm(AEnemySpawner* Spawner);
	/** GDS + EnemySpawner 기반 프리웜 요청 배열 구성 */
	TArray<FPoolPreWarmRequest> BuildPreWarmList(AEnemySpawner* Spawner);
	/** 프리웜 완료 핸들러 — 입력 복구 + 스테이지 시작 */
	void OnPreWarmCompleted();
	/** Tick에서 LoadingWidget 진행률 반영 */
	void UpdatePreWarmProgress();
	void StartStageFlow();
	void InitDefaultWeapon(FName CharID);

	/** EUIID::LOADING 위젯 조회 헬퍼 — nullptr 가능 */
	URSLoadingWidget* GetLoadingWidget() const;
	/** LoadingWidget FinishLoading 후 UMS를 통해 닫기 */
	void CloseLoadingUI();
	/** GDS 기반 현재 스테이지 웨이브에서 고유 에너미 클래스 수집 */
	TSet<TSubclassOf<AActor>> CollectUniqueEnemyClasses() const;
	/** FPoolPreWarmRequest 생성 헬퍼 */
	FPoolPreWarmRequest MakeActorRequest(TSubclassOf<AActor> Class, int32 Count);
	FPoolPreWarmRequest MakeWidgetRequest(TSubclassOf<UUserWidget> Class, int32 Count);

	// ── 스테이지 판정 ─────────────────────────────────────────────────────────
	void CheckStageClearCondition();
	void OnStageCleared();
	void EndStage(bool bCleared);
	void StopStageActivities();
	FStageResultData BuildResultData(bool bCleared);
	void SaveResult(const FStageResultData& ResultData);
	void ShowResultUI(const FStageResultData& ResultData, bool bCleared);

	UFUNCTION()
	void OnResultConfirmed();

private:
	/** 레벨에 배치된 EnemySpawner 캐시 */
	UPROPERTY()
	AEnemySpawner* CachedSpawner = nullptr;
	
	/** 프리웜 진행 중 플래그 — Tick에서 LoadingWidget 폴링 제어 */
	bool bIsPreWarmActive = false;
	
	/** SpawnPreview 스킬 프리뷰 액터 클래스 — BP에서 할당 (BP_SummonPreviewObject) */
	UPROPERTY(EditDefaultsOnly, Category = "MY|Skill")
	TSubclassOf<ASummonPreviewObject> PreviewActorClass;

	/** 데미지 플로팅 위젯 프리웜 클래스 — BP에서 할당 */
	UPROPERTY(EditDefaultsOnly, Category = "MY|PreWarm")
	TSubclassOf<UUserWidget> DamageFloatingWidgetClass;
	/** 데미지 플로팅 위젯 프리웜 수량 */
	UPROPERTY(EditDefaultsOnly, Category = "MY|PreWarm")
	int32 DamageFloatingWidgetPoolCount = 20;

	/** 스테이지 시작 시간 (GetWorld()->GetTimeSeconds() 기준) */
	float StageStartTime = 0.f;
	/** 현재 진행 중인 스테이지 ID */
	FName CurrentStageID = NAME_None;
	/** 스테이지 종료 플래그 (중복 호출 방지) */
	bool bIsStageEnded = false;
};
