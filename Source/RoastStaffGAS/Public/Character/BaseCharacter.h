// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "BaseCharacter.generated.h"

class UAbilitySystemComponent;
class UBaseAttributeSet;
class UGameplayEffect;

UCLASS()
class ROASTSTAFFGAS_API ABaseCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()
public:
	ABaseCharacter();

protected:
	virtual void BeginPlay() override;

	// -------------------------------------------------------------------------
	// GAS 초기화 — 자식 클래스가 반드시 구현 (Owner/Avatar 세팅은 자식 책임)
	// -------------------------------------------------------------------------
	virtual void InitializeAbilitySystem() PURE_VIRTUAL(ABaseCharacter::InitializeAbilitySystem,);
	
	// 자식 클래스가 자신의 AS를 반환 — 델리게이트 바인딩에 사용
	virtual UBaseAttributeSet* GetBaseAttributeSet() const	{ return nullptr; }
	
	// 피격 상태 처리 (Hit Montage 재생)
	// virtual void OnTakeHit();
	// 무적 상태 처리
	// virtual void OnImmune();
	// 공통 사망 상태 처리 프로세스
	UFUNCTION()
	virtual void HandleDeath();
	
	// AS 델리게이트 바인딩 — 자식이 AS 등록 완료 후 호출
	void BindAttributeDelegates();
	// 패시브 GE 적용 — 공통 로직
	void ApplyPassiveEffects(const TArray<TSoftClassPtr<UGameplayEffect>>& DefaultEffects);

public:
	// IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override	{ return nullptr; }

protected:
	// 사망 중복 방지 플래그
	bool bIsDead = false;

	
};
