// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayAbilitySpec.h"
#include "Data/RuntimeDataStructs.h"
#include "SkillManagerSubsystem.generated.h"

class UAbilitySystemComponent;
class URSCharacterSkillData;
class ASummonPreviewObject;

/** 슬롯별 런타임 상태 */
struct FSkillSlotState
{
	FCharacterSkillExecData ExecData;
	FGameplayAbilitySpecHandle Handle;
	FTimerHandle CooldownTimer;
	bool bIsOnCooldown = false;
	float CooldownRemaining = 0.f;
	float TotalCooldown = 0.f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSkillSlotUpdated, int32, SlotIndex);

/**
 * USkillManagerSubsystem
 * 캐릭터 고유 스킬 슬롯 관리 (Slot1=Q, Slot2=E)
 *
 */
UCLASS()
class ROASTSTAFFGAS_API USkillManagerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** 슬롯 변경(초기화/쿨타임 시작·종료) 시 SlotIndex 브로드캐스트 */
	UPROPERTY(BlueprintAssignable)
	FOnSkillSlotUpdated OnSkillSlotUpdatedDel;

	/** RSPlayerCharacter::InitializeAbilitySystem()에서 호출 */
	void InitializeSkills(FName CharacterID, UAbilitySystemComponent* InASC);
	/** Q(Slot=0) / E(Slot=1) 입력 진입점 */
	void ActivateSkillSlot(int32 SlotIndex);
	/** LMB 확정 — 프리뷰 파괴 + GA 발동 + 쿨타임 시작 */
	void ConfirmSkillPreview(FVector WorldLocation);
	/** RMB 취소 — 프리뷰 파괴, 쿨타임 없음 */
	void CancelSkillPreview();
	/** GA_CharacterSkill이 OnAbilityActivated에서 ExecData 조회 시 사용 */
	const FCharacterSkillExecData& GetSlotExecData(int32 SlotIndex) const;
	/** UI가 슬롯 상태(쿨타임 데이터 등) 읽을 때 사용 */
	const FSkillSlotState* GetSkillSlotState(int32 SlotIndex) const;

	/** 프리뷰 활성 여부 — RSPlayerController::OnConfirm 분기에 사용 */
	FORCEINLINE bool IsPreviewActive() const { return ActivePreviewSlot >= 0; }
	/** SpawnPreview 확정 위치 — GA_CharacterSkill::ExecuteSpawnPreview에서 읽음 */
	FORCEINLINE FVector GetPendingTargetLocation() const { return PendingTargetLocation; }

private:
	void SpawnPreviewActor(int32 SlotIndex);
	void DestroyPreviewActor();
	void StartCooldown(int32 SlotIndex);

	
private:
	static constexpr int32 SKILL_SLOT_COUNT = 2; // Q=0, E=1

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> ASC;
	/** GC 방지 — GiveAbility에 주입한 SourceObject 보관 */
	UPROPERTY()
	TArray<TObjectPtr<URSCharacterSkillData>> SkillDataObjects;

	/** 슬롯별 런타임 상태 */
	FSkillSlotState SkillSlots[SKILL_SLOT_COUNT];
	/** 현재 활성 프리뷰 슬롯 인덱스. -1이면 비활성 */
	int32 ActivePreviewSlot = -1;
	/** SpawnPreview 확정 위치 — ConfirmSkillPreview에서 세팅, GA에서 읽음 */
	FVector PendingTargetLocation = FVector::ZeroVector;

	UPROPERTY()
	TObjectPtr<ASummonPreviewObject> ActivePreviewActor;

	bool bIsInitialized = false;

	// 빈 ExecData — GetSlotExecData 범위 초과 시 반환용
	static const FCharacterSkillExecData EmptyExecData;
};
