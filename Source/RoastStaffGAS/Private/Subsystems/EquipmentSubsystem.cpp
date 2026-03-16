// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystems/EquipmentSubsystem.h"
#include "RoastStaffGAS.h"
#include "Subsystems/GameDataSubsystem.h"
#include "Data/RuntimeDataStructs.h"
#include "GAS/Tags/RSGameplayTags.h"
#include "Objects/Data/RSSkillData.h"
#include "System/LoggingSystem.h"

void UEquipmentSubsystem::InitializeSubsystem(UAbilitySystemComponent* InASC)
{
	if (bIsInitialized)
	{
		KHS_WARN(TEXT("이미 초기화됨. 중복 호출 무시."));
		return;
	}

	if (!ensureMsgf(InASC, TEXT("ASC가 null")))
	{
		return;
	}

	ASC = InASC;

	for (int32 i = 0; i < SLOT_COUNT; ++i)
	{
		Slots[i].SlotIndex = i;
	}

	bIsInitialized = true;
	KHS_INFO(TEXT("EquipmentSubsystem 초기화 완료"));
}

void UEquipmentSubsystem::RequestManualFire(const FVector& AimLocation)
{
	if (ActiveSlotIndex == -1)
	{
		return;
	}

	FireSlot(ActiveSlotIndex, AimLocation);
}

void UEquipmentSubsystem::RequestSlotActivate(int32 SlotIndex)
{
	if (!IsValidSlotIndex(SlotIndex))
	{
		return;
	}

	if (Slots[SlotIndex].IsEmpty())
	{
		KHS_INFO(TEXT("Slot %d 비어있음. 입력 무시."), SlotIndex);
		return;
	}

	if (ActiveSlotIndex == SlotIndex)
	{
		ClearActiveSlot();
		return;
	}

	SetSlotActive(SlotIndex);
}

void UEquipmentSubsystem::EquipWeapon(const FName& WeaponID)
{
	if (!ensureMsgf(ASC, TEXT("ASC IS NULL")))
    {
        return;
    }

	GET_GI_SUBSYSTEM(UGameDataSubsystem, GDS);
	
    FWeaponEquipData EquipData;
    if (!GDS->GetWeaponEquipData(WeaponID, EquipData))
    {
        KHS_WARN(TEXT("WeaponID 조회 실패: %s"), *WeaponID.ToString());
        return;
    }

    int32 TargetSlot = GetEmptySlotIndex();
    if (TargetSlot == INDEX_NONE)
    {
        KHS_INFO(TEXT("빈 슬롯 없음. 무기 획득 불가: %s"), *WeaponID.ToString());
        return;
    }

	//GA/GE클래스 로드
    TSubclassOf<UGameplayAbility> GAClass;
	if (!LoadRequiredClass(EquipData.GAClass, GAClass, WeaponID))
	{
		return;
	}
	
	TSubclassOf<UGameplayEffect> DamageGEClass;
	if (!LoadRequiredClass(EquipData.DamageGEClass, DamageGEClass, WeaponID))
	{
		return;
	}
	
	TSubclassOf<UGameplayEffect> StatusGEClass = LoadOptionalClass(EquipData.StatusGEClass, WeaponID);
	TSubclassOf<AActor> ProjectileClass = LoadOptionalClass(EquipData.ProjectileClass, WeaponID);
	

	//GA 이벤트 발동 시 SkillID 함께 전달하기 위한 DTO추가
    URSSkillData* SkillDataObj = NewObject<URSSkillData>(this);
    SkillDataObj->SkillID = EquipData.SkillID;
    SkillDataObjects.Add(SkillDataObj);

    FGameplayAbilitySpec Spec(GAClass, 1, INDEX_NONE, SkillDataObj);
    FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(Spec);
    if (!ensureMsgf(Handle.IsValid(), TEXT("GiveAbility 실패: %s"), *WeaponID.ToString()))
    {
        return;
    }

	//슬롯데이터 관리.(GA와 슬롯은 서로 모르니까)
    FWeaponSlotInstanceData& Slot  = Slots[TargetSlot];
    Slot.EquipData.WeaponID        = WeaponID;
    Slot.EquipData.SkillID         = EquipData.SkillID;
    Slot.AbilitySpecHandle         = Handle;
    Slot.EquipData.Cooldown        = EquipData.Cooldown;
    Slot.CooldownRemaining         = 0.f;
    Slot.bIsActive                 = false;
    Slot.EquipData.GAClass         = GAClass;
    Slot.EquipData.ProjectileClass = ProjectileClass;
    Slot.EquipData.DamageGEClass   = DamageGEClass;
    Slot.EquipData.StatusGEClass   = StatusGEClass;

    KHS_INFO(TEXT("무기 장착 완료: %s → Slot %d"), *WeaponID.ToString(), TargetSlot);

    StartAutoFire(TargetSlot);
	
	//이벤트 발행
    OnSlotUpdatedDel.Broadcast(TargetSlot);
}

void UEquipmentSubsystem::StopAllFire()
{
	for (FWeaponSlotInstanceData& Slot : Slots)
	{
		if (Slot.AutoFireTimerHandle.IsValid())
		{
			GetGameInstance()->GetWorld()->GetTimerManager().ClearTimer(Slot.AutoFireTimerHandle);
		}
	}

	KHS_INFO(TEXT("모든 슬롯 자동발사 타이머 클리어."));
}

const FWeaponSlotInstanceData* UEquipmentSubsystem::GetSlotData(int32 SlotIndex) const
{
	if (!IsValidSlotIndex(SlotIndex))
	{
		return nullptr;
	}

	return &Slots[SlotIndex];
}

void UEquipmentSubsystem::FireSlot(int32 SlotIndex, const FVector& AimLocation)
{
	FWeaponSlotInstanceData& Slot = Slots[SlotIndex];

	FGameplayEventData Payload;
	Payload.Instigator = ASC->GetAvatarActor();
	Payload.Target     = nullptr;
	// TODO: AimLocation → TargetData로 전달 (GA 구현 시 확정)

	const FGameplayTag EventTag = GetEventTag(Slot);
	if (EventTag == FGameplayTag::EmptyTag)
	{
		KHS_WARN(TEXT("Slot %d: 유효하지 않은 EventTag"), SlotIndex);
		return;
	}

	ASC->TriggerAbilityFromGameplayEvent(Slot.AbilitySpecHandle,
		ASC->AbilityActorInfo.Get(), EventTag, &Payload,*ASC);

	KHS_INFO(TEXT("Slot %d: %s 발사! CD: %.2fs"), SlotIndex, *Slot.EquipData.SkillID.ToString(), Slot.EquipData.Cooldown);
}

void UEquipmentSubsystem::StartAutoFire(int32 SlotIndex)
{
	FWeaponSlotInstanceData& Slot = Slots[SlotIndex];

	// 자동발사는 에임 좌표 없이 발사 — FVector::ZeroVector로 전달
	// TODO: 자동발사 에임 방향 전략 (가장 가까운 적 방향 등) — 적 시스템 구현 후 결정
	GetGameInstance()->GetWorld()->GetTimerManager().SetTimer(
		Slot.AutoFireTimerHandle,
		[this, SlotIndex]()
		{
			if (!Slots[SlotIndex].bIsActive)
			{
				FireSlot(SlotIndex, FVector::ZeroVector);
			}
		},
		Slot.EquipData.Cooldown, true, 0.f);

	KHS_INFO(TEXT("Slot %d: 자동공격 타이머 시작. CD: %.2fs"),	SlotIndex, Slot.EquipData.Cooldown);
}

void UEquipmentSubsystem::StopAutoFire(int32 SlotIndex)
{
	GetGameInstance()->GetWorld()->GetTimerManager().ClearTimer(Slots[SlotIndex].AutoFireTimerHandle);
}

void UEquipmentSubsystem::SetSlotActive(int32 SlotIndex)
{
	if (ActiveSlotIndex != -1)
	{
		Slots[ActiveSlotIndex].bIsActive = false;
		StartAutoFire(ActiveSlotIndex);
		OnSlotUpdatedDel.Broadcast(ActiveSlotIndex);
	}

	Slots[SlotIndex].bIsActive = true;
	ActiveSlotIndex = SlotIndex;
	StopAutoFire(SlotIndex);

	KHS_INFO(TEXT("Slot %d → 액티브 모드 전환."), SlotIndex);
	OnSlotUpdatedDel.Broadcast(SlotIndex);
}

void UEquipmentSubsystem::ClearActiveSlot()
{
	if (ActiveSlotIndex == -1)
	{
		return;
	}

	Slots[ActiveSlotIndex].bIsActive = false;
	StartAutoFire(ActiveSlotIndex);

	KHS_INFO(TEXT("Slot %d → 자동공격 복귀."), ActiveSlotIndex);
	OnSlotUpdatedDel.Broadcast(ActiveSlotIndex);

	ActiveSlotIndex = -1;
}

bool UEquipmentSubsystem::IsValidSlotIndex(int32 SlotIndex) const
{
	return SlotIndex >= 0 && SlotIndex < SLOT_COUNT;
}

int32 UEquipmentSubsystem::GetEmptySlotIndex() const
{
	for (int32 i = 0; i < SLOT_COUNT; ++i)
	{
		if (Slots[i].IsEmpty())
		{
			return i;
		}
	}

	return INDEX_NONE;
}

FGameplayTag UEquipmentSubsystem::GetEventTag(const FWeaponSlotInstanceData& Slot) const
{
	switch (Slot.EquipData.SkillType)
	{
	case ESkillType::PROJECTILE:
		return RSTags::Event_Weapon_Fire_Projectile;
	case ESkillType::SUMMON:
		return RSTags::Event_Weapon_Fire_Summon;
	default:
		{
			KHS_WARN(TEXT("Invalid GameplayTag"));
			return FGameplayTag::EmptyTag;
		}
	}
}
