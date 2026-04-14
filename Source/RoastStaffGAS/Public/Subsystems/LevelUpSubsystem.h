// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/RuntimeDataStructs.h"
#include "LevelUpSubsystem.generated.h"

/** 카드풀 선정 완료 — PlayerController가 구독하여 레벨업 UI 오픈 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCardPoolReady, const TArray<FLevelUpCardDisplayData>&, Cards);

class UAbilitySystemComponent;
class UPlayerAttributeSet;
class UGameplayEffect;

UCLASS()
class ROASTSTAFFGAS_API ULevelUpSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** PlayerCharacter::InitializeAbilitySystem()에서 호출 */
	void InitializeSubsystem(UAbilitySystemComponent* InASC,UPlayerAttributeSet* InAttributeSet,TSubclassOf<UGameplayEffect> InAddEXPEffectClass);
	
	UFUNCTION(BlueprintCallable, Category = "MY|LevelUp")
	void AddEXP(float Amount);
	/** AEnemyBaseCharacter::OnEnemyKilledDel → StageManagerSubsystem 경유로 호출 */
	UFUNCTION()
	void OnEnemyKilled(FName InEnemyID);
	/** PlayerController가 UI 종료 시 호출 — bIsLevelingUp 해제 */
	void NotifyWeaponSelectCompleted();
	/** 레벨업 UI에서 카드 선택 시 호출 — 타입별 효과 적용 */
	void OnCardSelected(FName CardID);

private:
	UFUNCTION()
	void OnEXPChanged(float NewEXP, int32 CurrentLevel);

	void CheckLevelUp(float NewEXP, int32 CurrentLevel);
	void ApplyLevelUp(int32 CurrentLevel, float OverflowEXP);

	/** 정적 + 동적 카드풀 통합 → PickFinalCards → OnCardPoolReadyDel 발행 */
	void BuildCardPool();
	void ApplyStatUpgrade(const FLevelUpCardStaticData& CardData);
	void EnsureWeaponCardGuarantee(TArray<FLevelUpCardDisplayData>& CardPool);
	
	TArray<FLevelUpCardDisplayData> BuildStaticCardPool();
	TArray<FLevelUpCardDisplayData> BuildDynamicWeaponCards();
	TArray<FLevelUpCardDisplayData> PickFinalCards(const TArray<FLevelUpCardDisplayData>& Pool);

public:
	/** 카드풀 선정 완료 시 발행 — PlayerController가 구독하여 레벨업 UI 오픈 */
	UPROPERTY(BlueprintAssignable, Category = "MY|LevelUp")
	FOnCardPoolReady OnCardPoolReadyDel;
	
private:
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> ASC;
	UPROPERTY()
	TObjectPtr<UPlayerAttributeSet> AttributeSet;
	UPROPERTY()
	TSubclassOf<UGameplayEffect> AddEXPEffectClass;

	bool bIsLevelingUp  = false;
	bool bIsInitialized = false;

	static constexpr int32 MAX_LEVEL       = 20;
	static constexpr int32 CARD_PICK_COUNT = 4;
};
