// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "K_BaseCharacter.generated.h"

class UGameplayAbility;
class UAbilitySystemComponent;
class UK_BaseAttributeSet;

/**
 * K_BaseCharacter
 * 
 * GAS를 사용하는 모든 캐릭터의 기본 클래스.
 * 
 * ASC 소유권 구조:
 * 
 * - AI/NPC: 이 클래스에서 직접 ASC를 생성하고 소유 (bUsePlayerStateASC = false)
 * - AI는 PlayerState가 없으므로 Character에서 직접 ASC 소유 필요
 * 
 * - Player: PlayerState에서 ASC를 가져옴 (K_PlayerCharacter에서 오버라이드)
 * - Player는 Pawn 교체 시에도 어빌리티 상태 유지를 위해 PlayerState에서 소유
 * 
 */

UCLASS()
class ROASTSTAFFGAS_API AK_BaseCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AK_BaseCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void PostInitializeComponents() override;

	//GAS 초기화
	virtual void InitializeAbilitySystem();
	
public:
	//IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	//AttributeSet Getter
	UFUNCTION(BlueprintCallable, Category = "AM|GAS")
	virtual UK_BaseAttributeSet* GetAttributeSet() const {return BaseAttributeSet;}
	
	UFUNCTION(BlueprintImplementableEvent)
	void MyPostInitailizeComponents();

protected:
	/* //////////////////////
	 * GAS Contents  - NPC / AI 사용
	 *///////////////////////
	
	//ASC - NPC / AI 사용
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AM|GAS")
	TObjectPtr<class UAbilitySystemComponent> ASC;
	
	//Attribute Sets - NPC / AI 사용
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AM|GAS")
	TObjectPtr<UK_BaseAttributeSet> BaseAttributeSet;
	
	//Game Abilities  - NPC / AI 사용
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AM|GAS")
	TArray<TSubclassOf<UGameplayAbility>> InitialAbilities;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AM|GAS")
	int32 CharacterLevel;
	
	bool bASCInitialized;
};
