// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "K_BaseGameplayAbility.generated.h"


class AK_BaseCharacter;
class UK_BaseAttributeSet;
/**
 * UK_BaseGameplayAbility
 * - 프로젝트의 모든 GA의 베이스 
 * 
 * 역할
 * - 캐릭터/ASC접근을 위한 헬퍼함수
 * - 공통 설정
 * - 디버깅/로깅 유틸리티
 */
UCLASS(Abstract)
class ROASTSTAFFGAS_API UK_BaseGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UK_BaseGameplayAbility();
	
protected:
	/*
	 * GA가 보유한 캐릭터 정보 반환
	 * @param : ActorInfo
	 * @return : 캐릭터 포인터, 실패시 nullptr
	 */
	UFUNCTION(BlueprintCallable, Category = "AM|Ability")
	AK_BaseCharacter* GetCharacterFromActorInfo(const FGameplayAbilityActorInfo& ActorInfo) const;
	
	/*
	 * GA 오너의 ASC정보 반환
	 */
	UFUNCTION(BlueprintCallable, Category = "AM|Ability")
	UAbilitySystemComponent* GetOwnerASC() const;
	
	/*
	 * GA 오너의 BaseAttributeSet 반환
	 */
	UFUNCTION(BlueprintCallable, Category = "AM|Ability")
	UK_BaseAttributeSet* GetOwnerBaseAttributeSet() const;
	
	//로그 헬퍼
	void LogAbilityInfo(const FString& msg) const;
	
protected:
	//GA 사용시 로그 출력 여부
	UPROPERTY(EditDefaultsOnly, Category = "AM|Ability")
	bool bLogActivated = true;
	
	
};
