// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "PassiveSlotSubsystem.generated.h"

/** 패시브 슬롯 상태 변경 시 발행 — PlayerController HUD 갱신용 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPassiveSlotChanged);

class UAbilitySystemComponent;

/**
 * UPassiveSlotSubsystem
 * 패시브 스킬 슬롯 관리 (최대 4슬롯, 영구 GE 적용)
 *
 */
UCLASS()
class ROASTSTAFFGAS_API UPassiveSlotSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** RSPlayerCharacter::InitializeAbilitySystem()에서 호출 */
	void InitializeSubsystem(UAbilitySystemComponent* InASC);
	/** 패시브 슬롯에 추가. 슬롯 포화 또는 GE 로드 실패 시 false 반환 */
	bool TryAddPassive(FName PassiveID);

	/** 슬롯이 꽉 찼는지 여부 — LevelUpSubsystem 카드 필터 진입점 */
	FORCEINLINE bool IsSlotFull() const { return EquippedPassiveIDs.Num() >= MAX_SLOTS; }
	FORCEINLINE TArray<FName> GetEquippedPassives() const { return EquippedPassiveIDs; }

	/** 슬롯 상태 변경 시 발행 — PlayerController 구독 */
	UPROPERTY(BlueprintAssignable, Category = "MY|Passive")
	FOnPassiveSlotChanged OnPassiveSlotChangedDel;

private:
	static constexpr int32 MAX_SLOTS = 4;

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> ASC;

	TArray<FName> EquippedPassiveIDs;
	bool bIsInitialized = false;
	
};
