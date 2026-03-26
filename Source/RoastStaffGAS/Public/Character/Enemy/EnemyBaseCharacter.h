// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/BaseCharacter.h"
#include "Data/DataTableStructs.h"
#include "Interface/PoolableInterface.h"
#include "EnemyBaseCharacter.generated.h"


/**
 * ABaseEnemyCharacter
 *
 * - 에너미 베이스 클래스.
 * - 스폰 직후 InitializeEnemy(EnemyID)를 반드시 호출
 * - 사망 시 OnEnemyKilledDel을 Broadcast → StageWaveSubsystem이 수신.
 */

class UEnemyAttributeSet;
class UAbilitySystemComponent;
class UGameDataSubsystem;
class UWidgetComponent;
class UEnemyHPBarWidget;
class UFloatingDamageWidget;

// 에너미 처치 델리게이트 — StageWaveSubsystem이 구독
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyKilled, FName, EnemyID);


UCLASS()
class ROASTSTAFFGAS_API AEnemyBaseCharacter : public ABaseCharacter, public IPoolableInterface
{
	GENERATED_BODY()

public:
	AEnemyBaseCharacter();

	// 스폰 직후 DT_Enemy 기반 스탯 주입 및 AI 시작
	UFUNCTION(BlueprintCallable, Category = "MY|Enemy")
	void InitializeEnemy(FName InEnemyID);

	FORCEINLINE FName GetEnemyID() const { return EnemyID; }

protected:
	virtual void BeginPlay() override;

	// ABaseCharacter 오버라이드
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual UBaseAttributeSet* GetBaseAttributeSet() const override;
	virtual void InitializeAbilitySystem() override;
	virtual void HandleDeath() override;

	// IPoolableInterface 구현
	virtual void OnPoolActivate() override;
	virtual void OnPoolDeactivate() override;

private:
	bool ApplyStatData(FEnemyStaticData& EnemyData);
	bool StartEnemyAI(FEnemyStaticData EnemyData);

	/** WidgetComponent에 HPBarWidget을 설정하고 ASC에 바인딩 */
	void SetupHPBar();

public:
	// 처치 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "MY|Enemy")
	FOnEnemyKilled OnEnemyKilledDel;

private:
	// ASC
	UPROPERTY(VisibleAnywhere, Category = "MY|GAS")
	TObjectPtr<UAbilitySystemComponent> ASC;
	// 에너미 전용 AS
	UPROPERTY(VisibleAnywhere, Category = "MY|GAS")
	TObjectPtr<UEnemyAttributeSet> EnemyAttributeSet;

	// UI 컴포넌트
	UPROPERTY(VisibleAnywhere, Category = "MY|UI")
	TObjectPtr<UWidgetComponent> HPBarWidgetComp;
	UPROPERTY(EditDefaultsOnly, Category = "MY|UI")
	TSubclassOf<UEnemyHPBarWidget> HPBarWidgetClass;

	// EnemyID 캐싱(초기화/사망 시 델리게이트 용도)
	FName EnemyID;
	// 중복 초기화 방지
	bool bIsInitialized = false;

	/** 사망 후 풀 반납까지 대기 시간 (초) — 사망 연출 길이에 맞춰 BP에서 조정 */
	UPROPERTY(EditDefaultsOnly, Category = "MY|Enemy")
	float DeathPoolReturnDelay = 2.f;

	/** 사망 후 풀 반납 타이머 */
	FTimerHandle DeathReturnTimerHandle;
};
