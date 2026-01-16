// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "K_PlayerState.generated.h"


class UK_BaseAttributeSet;
class UAbilitySystemComponent;
class UGameplayAbility;
/**
*  K_PlayerState
 * 
 * 멀티플레이어 환경에서 ASC를 소유하는 PlayerState 클래스.
 */
UCLASS()
class ROASTSTAFFGAS_API AK_PlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:
	AK_PlayerState();
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	UFUNCTION(BlueprintCallable, Category = "AM|GAS")
	UK_BaseAttributeSet* GetBaseAttributeSet() const {return BaseAttributeSet; }

	//초기 어빌리티 부여(Possess 시점)
	void InitializeAbilities(AActor* AvatarActor);

private:
	//중복 초기화 방지 플래그
	bool bAbilitiesInitialized = false;
	
protected:
	//ASC
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AM|GAS")
	TObjectPtr<UAbilitySystemComponent> ASC;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AM|GAS")
	TObjectPtr<UK_BaseAttributeSet> BaseAttributeSet;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AM|GAS")
	TArray<TSubclassOf<UGameplayAbility>> InitialAbilities;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AM|GAS")
	int32 CharacterLevel = 1;
	
};
