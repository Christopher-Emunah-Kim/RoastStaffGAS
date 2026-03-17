// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/BaseCharacter.h"
#include "RSPlayerCharacter.generated.h"

class ARSPlayerState;
class USpringArmComponent;
class UCameraComponent;
class UEquipmentComponent;

UCLASS()
class ROASTSTAFFGAS_API ARSPlayerCharacter : public ABaseCharacter
{
	GENERATED_BODY()
	
	public:
    ARSPlayerCharacter();

protected:
    virtual void BeginPlay() override;
    virtual void PossessedBy(AController* NewController) override;
    
    // ABaseCharacter 오버라이드
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override; // PlayerState의 ASC 
    virtual UBaseAttributeSet* GetBaseAttributeSet() const override; // PlayerState의 PlayerAttributeSet 반환
    virtual void InitializeAbilitySystem() override; // Owner = PlayerState, Avatar = this
    virtual void HandleDeath() override; // 고유 사망 처리
    
private:
    ARSPlayerState* GetRSPlayerState() const;

protected:
    // 컴포넌트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<USpringArmComponent> SpringArm;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<UCameraComponent> Camera;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<UEquipmentComponent> EquipmentComp;
    
    //캐릭터 경험치 GE
    UPROPERTY(EditDefaultsOnly, Category = "MY|LevelUp")
    TSubclassOf<UGameplayEffect> AddEXPEffectClass;
    
};
