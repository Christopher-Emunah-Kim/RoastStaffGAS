// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/RuntimeDataStructs.h"
#include "LevelUpSubsystem.generated.h"

/** 무기 후보 선정 완료 — PlayerController가 구독하여 UI 오픈 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponCandidatesReady, const TArray<FWeaponCardDisplayData>&, WeaponCards);

class UAbilitySystemComponent;
class UPlayerAttributeSet;
class UGameplayEffect;

UCLASS()
class ROASTSTAFFGAS_API ULevelUpSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** PlayerCharacter::InitializeAbilitySystem()에서 호출 */
	void InitializeSubsystem(
		UAbilitySystemComponent* InASC,
		UPlayerAttributeSet* InAttributeSet,
		TSubclassOf<UGameplayEffect> InAddEXPEffectClass);

	UFUNCTION(BlueprintCallable, Category = "MY|LevelUp")
	void AddEXP(float Amount);

	/** 무기 후보 선정 완료 시 발행 — PlayerController가 구독하여 레벨업 UI 오픈 */
	UPROPERTY(BlueprintAssignable, Category = "MY|LevelUp")
	FOnWeaponCandidatesReady OnWeaponCandidatesReadyDel;

	/** PlayerController가 UI 종료 시 호출 — bIsLevelingUp 해제 */
	void NotifyWeaponSelectCompleted();

private:
	UFUNCTION()
	void OnEXPChanged(float NewEXP, int32 CurrentLevel);

	void CheckLevelUp(float NewEXP, int32 CurrentLevel);
	void SelectWeaponCandidates();
	void ApplyLevelUp(int32 CurrentLevel, float OverflowEXP);

private:
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> ASC;
	UPROPERTY()
	TObjectPtr<UPlayerAttributeSet> AttributeSet;
	UPROPERTY()
	TSubclassOf<UGameplayEffect> AddEXPEffectClass;

	bool bIsLevelingUp  = false;
	bool bIsInitialized = false;

	static constexpr int32 MAX_LEVEL            = 20;
	static constexpr int32 WEAPON_CANDIDATE_COUNT = 3;
};
