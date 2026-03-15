// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LevelUpComponent.generated.h"

// //무기 후보 선정 완료 델리게이트(선정된 무기 ID 3종)
// DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponCandidatesReady, const TArray<FName>&, WeaponIDs);
//
// class UAbilitySystemComponent;
// class UPlayerAttributeSet;
// class UEquipmentComponent;
// class UGameplayEffect;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ROASTSTAFFGAS_API ULevelUpComponent : public UActorComponent
{
	GENERATED_BODY()

//
// public:
// 	ULevelUpComponent();
//
// 	void Initialize(UAbilitySystemComponent* InASC, UPlayerAttributeSet* InAttributeSet, UEquipmentComponent* InEquipmentComp);
// 	
// 	UFUNCTION(BlueprintCallable, Category = "MY|LevelUp")
// 	void AddEXP(float Amount);
//
// 	// 무기 후보 선정 완료 시 발행
// 	UPROPERTY(BlueprintAssignable, Category = "MY|LevelUp")
// 	FOnWeaponCandidatesReady OnWeaponCandidatesReadyDel;
//
// private:
// 	// EXP 변경 감지 콜백
// 	UFUNCTION()
// 	void OnEXPChanged(float NewEXP, int32 CurrentLevel);
// 	
// 	// 레벨업 판정 및 처리
// 	void CheckLevelUp(float NewEXP, int32 CurrentLevel);
// 	// 무기 후보 3종 랜덤 선정
// 	void SelectWeaponCandidates();
// 	// 레벨 어트리뷰트 GE로 증가
// 	void ApplyLevelUp(int32 CurrentLevel, float OverflowEXP);
//
// private:
// 	UPROPERTY()
// 	TObjectPtr<UAbilitySystemComponent> ASC;
// 	UPROPERTY()
// 	TObjectPtr<UPlayerAttributeSet> AttributeSet;
// 	UPROPERTY()
// 	TObjectPtr<UEquipmentComponent> EquipmentComp;
// 	UPROPERTY(EditDefaultsOnly, Category = "MY|LevelUp")
// 	TSubclassOf<UGameplayEffect> AddEXPEffectClass;
// 	
// 	// 레벨업 이벤트 중복 방지
// 	bool bIsLevelingUp = false;
//
// 	// 임시 최대레벨
// 	static constexpr int32 MAX_LEVEL = 20;
	
	
};
