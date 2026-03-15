// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "LevelUpSubsystem.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponCandidatesReady, const TArray<FName>&, WeaponIDs);

class UAbilitySystemComponent;
class UPlayerAttributeSet;
class UGameplayEffect;

UCLASS()
class ROASTSTAFFGAS_API ULevelUpSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// PlayerCharacter::InitializeAbilitySystem()에서 호출
	void InitializeSubsystem(UAbilitySystemComponent* InASC, UPlayerAttributeSet* InAttributeSet, TSubclassOf<UGameplayEffect> InAddEXPEffectClass);

	UFUNCTION(BlueprintCallable, Category = "MY|LevelUp")
	void AddEXP(float Amount);

private:
	UFUNCTION()
	void OnEXPChanged(float NewEXP, int32 CurrentLevel);

	void CheckLevelUp(float NewEXP, int32 CurrentLevel);
	void SelectWeaponCandidates();
	void ApplyLevelUp(int32 CurrentLevel, float OverflowEXP);

public:
	// 무기 후보 선정 완료 시 발행 — UI 또는 EquipmentSubsystem이 구독
	UPROPERTY(BlueprintAssignable, Category = "MY|LevelUp")
	FOnWeaponCandidatesReady OnWeaponCandidatesReadyDel;

private:
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> ASC;
	UPROPERTY()
	TObjectPtr<UPlayerAttributeSet> AttributeSet;
	UPROPERTY()
	TSubclassOf<UGameplayEffect> AddEXPEffectClass;

	bool bIsLevelingUp = false;
	bool bIsInitialized = false;

	static constexpr int32 MAX_LEVEL = 20;
	static constexpr int32 WEAPON_CANDIDATE_COUNT = 3;
	
	
};
