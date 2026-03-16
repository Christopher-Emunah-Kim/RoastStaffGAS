// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "RSPlayerState.generated.h"


/**
 * RSPlayerState
 *
 * - ASC 소유권을 PlayerState가 가짐
 *   → 플레이어 사망/리스폰 시에도 레벨, 무기 슬롯 정보 유지
 * - PlayerAttributeSet을 생성하고 ASC에 등록
 * - PlayerCharacter가 PossessedBy 시점에 InitializeAbilitySystem()을 호출
 */

class UAbilitySystemComponent;
class UPlayerAttributeSet;

UCLASS()
class ROASTSTAFFGAS_API ARSPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:
	ARSPlayerState();

private:
	// 스탯 기본값 주입 헬퍼
	void ApplyBaseStats();
	
public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return ASC; }
	
	UPlayerAttributeSet* GetPlayerAttributeSet() const { return PlayerAttributeSet; }
	//PlayerCharacter::InitializeAbilitySystem()에서 호출
	void InitializeAbilitySystem(AActor* AvatarActor);  //Owner = PlayerState, Avatar = PlayerCharacter

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MY|GAS")
	TObjectPtr<UAbilitySystemComponent> ASC;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MY|GAS")
	TObjectPtr<UPlayerAttributeSet> PlayerAttributeSet;
	
private:
	// 중복 초기화 방지
	bool bIsInitialized = false;
	
};
