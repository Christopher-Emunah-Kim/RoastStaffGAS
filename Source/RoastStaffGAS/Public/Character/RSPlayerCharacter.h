// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/BaseCharacter.h"
#include "RSPlayerCharacter.generated.h"

class ARSPlayerState;
class USpringArmComponent;
class UCameraComponent;
class UEquipmentComponent;
class ULevelUpComponent;
class UInputAction;
class UInputMappingContext;
struct FInputActionValue;

UCLASS(PrioritizeCategories = ("MY|Input"))
class ROASTSTAFFGAS_API ARSPlayerCharacter : public ABaseCharacter
{
	GENERATED_BODY()
	
	public:
    ARSPlayerCharacter();

protected:
    virtual void BeginPlay() override;
    virtual void PossessedBy(AController* NewController) override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

    // ABaseCharacter 오버라이드
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override; // PlayerState의 ASC 
    virtual UBaseAttributeSet* GetBaseAttributeSet() const override; // PlayerState의 PlayerAttributeSet 반환
    virtual void InitializeAbilitySystem() override; // Owner = PlayerState, Avatar = this
    virtual void HandleDeath() override; // 고유 사망 처리
    
    // GA 발동 헬퍼
    //void TryActivateDash();
    
private:
    ARSPlayerState* GetRSPlayerState() const;
    
    // 입력 핸들러
    bool HandleMouseAim();
    
    void OnMove(const FInputActionValue& Value);
    void OnMouseAim(const FInputActionValue& Value);
    //void OnDash(const FInputActionValue& Value);
    void OnShootStart(const FInputActionValue& Value);
    void OnSlotActivate(const FInputActionValue& Value, int32 SlotIndex);

protected:
    // 컴포넌트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<USpringArmComponent> SpringArm;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<UCameraComponent> Camera;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<UEquipmentComponent> EquipmentComp;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<ULevelUpComponent> LevelUpComp;
    
    // 입력 에셋
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MY|Input")
    TObjectPtr<UInputMappingContext> IMC;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MY|Input")
    TObjectPtr<UInputAction> IA_Move;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MY|Input")
    TObjectPtr<UInputAction> IA_MouseAim;
    //UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MY|Input")
    //TObjectPtr<UInputAction> IA_Dash;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MY|Input")
    TObjectPtr<UInputAction> IA_Attack;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MY|Input")
    TObjectPtr<UInputAction> IA_Slot1;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MY|Input")
    TObjectPtr<UInputAction> IA_Slot2;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MY|Input")
    TObjectPtr<UInputAction> IA_Slot3;
    
    // 런타임 상태
    // 마지막 이동 입력 — 대시 방향 계산용
    FVector2D LastMoveInput = FVector2D::ZeroVector;
    // 에임 각도 (Yaw)
    float AimAngle = 0.f;
    
};
