// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystems/EquipmentSubsystem.h"
#include "RoastStaffGAS.h"
#include "Subsystems/GameDataSubsystem.h"
#include "Data/RuntimeDataStructs.h"
#include "GAS/Tags/RSGameplayTags.h"
#include "Objects/Data/RSSkillData.h"
#include "System/LoggingSystem.h"
#include "Objects/Projectile/BaseProjectile.h"

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

	//슬롯 스킬이 소환형인경우 -> 클릭 = 에임 장판 확인 후 ASC Confirm 전달
	if (Slots[ActiveSlotIndex].SlotEquipData.MoveType == EMoveType::SUMMON)
	{
		if (ASC)
		{
			ASC->LocalInputConfirm();
		}
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

	// Step 1: 동일 BaseType 슬롯 강화 체크
	FWeaponStaticData IncomingData;
	if (GDS->GetWeaponData(WeaponID, IncomingData))
	{
		for (int32 i = 0; i < SLOT_COUNT; ++i)
		{
			if (Slots[i].IsEmpty()) continue;

			FWeaponStaticData SlotData;
			if (!GDS->GetWeaponData(Slots[i].SlotEquipData.WeaponID, SlotData)) continue;

			if (SlotData.BaseType == IncomingData.BaseType && SlotData.NextLevelWeaponID != NAME_None)
			{
				UpgradeWeapon(i, SlotData.NextLevelWeaponID);
				return;
			}
		}
	}

	// Step 2: 빈 슬롯 장착
	int32 TargetSlot = GetEmptySlotIndex();
	if (TargetSlot != INDEX_NONE)
	{
		FWeaponSlotEquipData EquipData;
		if (!LoadEquipData(WeaponID, EquipData))
		{
			return;
		}

		FLoadedEquipClasses Classes;
		if (!LoadEquipClasses(EquipData, Classes))
		{
			return;
		}

		FGameplayAbilitySpecHandle Handle;
		if (!RegisterAbility(EquipData, Classes, Handle))
		{
			return;
		}

		CommitSlot(TargetSlot, EquipData, Handle);
		return;
	}

	// Step 3: 슬롯 가득 + 강화 불가 → 교체 UI 신호
	KHS_INFO(TEXT("슬롯 가득 + 강화 불가. 교체 UI 대기: %s"), *WeaponID.ToString());
	PendingWeaponID = WeaponID;
	OnSlotFull.Broadcast(WeaponID);
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

void UEquipmentSubsystem::OnSummonAbilityEnded(FName SkillID)
{
	for (int32 i = 0; i < SLOT_COUNT; ++i)                                                                       
	{                                                                                                            
		if (Slots[i].SlotEquipData.SkillID != SkillID)
		{                                                                                                        
	          continue;                                                                                          
		}                                                                                                        
	                                                                                                               
		const float Cooldown = FMath::Max(0.1f, Slots[i].SlotEquipData.Cooldown);                                
	                         
		KHS_INFO(TEXT("[EQS] OnSummonAbilityEnded. SkillID: %s, bIsActive: %s"), *SkillID.ToString(), Slots[i].bIsActive ? TEXT("true") : TEXT("false"));  
	    if (Slots[i].bIsActive)                                                                                  
	    {                                                                                                      
	        // Active 모드: GA 재발동 (에임 장판 프리뷰 재시작)                                
	    	// EndAbility 완전 종료 후 다음 틱에 FireSlot (GA 종료 완료 보장)
	    	GetGameInstance()->GetWorld()->GetTimerManager().SetTimer(
	    		Slots[i].AutoFireTimerHandle, [this, i]()                                                                                              
	    		{                                                                                                      
	  	  			if (IsValidSlotIndex(i) && Slots[i].bIsActive)                                                       
	  			    {                                                                                                    
	  				  FireSlot(i, FVector::ZeroVector);
	  			    } 
	    		},  0.1f, false); 
	    }        
	    else                                                                                                   
	    {                                                                                                        
	      // 자동 모드: 쿨타임 후 반복 발동 재시작 (첫 딜레이 = Cooldown)
			GetGameInstance()->GetWorld()->GetTimerManager().SetTimer(
				Slots[i].AutoFireTimerHandle, [this, i]() 
				{                                                                                                
				  if (!Slots[i].bIsActive)                                                                   
				  {
					  FireSlot(i, FVector::ZeroVector);
				  }                                                                                            
				}, Cooldown, true, Cooldown);  //쿨타임마다 루프                                                                 
	    }                                                                                                        
		break;                                                                                                   
	}                                                                                                            
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
	SetGameplayEventData(AimLocation, Payload);

	const FGameplayTag EventTag = GetEventTag(Slot);
	if (EventTag == FGameplayTag::EmptyTag)
	{
		KHS_WARN(TEXT("Slot %d: 유효하지 않은 EventTag"), SlotIndex);
		return;
	}

	ASC->TriggerAbilityFromGameplayEvent(Slot.AbilitySpecHandle,
		ASC->AbilityActorInfo.Get(), EventTag, &Payload,*ASC);

	//UI업데이트
	Slot.CooldownRemaining = Slot.SlotEquipData.Cooldown;
	OnSlotUpdatedDel.Broadcast(SlotIndex);

	KHS_INFO(TEXT("Slot %d: %s 발사! CD: %.2fs"), SlotIndex, *Slot.SlotEquipData.SkillID.ToString(), Slot.SlotEquipData.Cooldown);
}


void UEquipmentSubsystem::SetGameplayEventData(const FVector& AimLocation, FGameplayEventData& Payload)
{
	FGameplayAbilityTargetData_LocationInfo* LocationData = new FGameplayAbilityTargetData_LocationInfo();
	LocationData->TargetLocation.LiteralTransform = FTransform(AimLocation);
	FGameplayAbilityTargetDataHandle TargetDataHandle;
	TargetDataHandle.Add(LocationData);
	
	Payload.Instigator = ASC->GetAvatarActor();
	Payload.Target     = nullptr;
	Payload.TargetData = TargetDataHandle;
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
		Slot.SlotEquipData.Cooldown, true, 0.f);

	KHS_INFO(TEXT("Slot %d: 자동공격 타이머 시작. CD: %.2fs"),	SlotIndex, Slot.SlotEquipData.Cooldown);
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

	// 소환형 스킬: 슬롯 활성화 즉시 GA 발동 (GA 내부에서 bIsActive로 모드 분기)                                 
	if (Slots[SlotIndex].SlotEquipData.MoveType == EMoveType::SUMMON)                                            
	{                                                                                                            
		FireSlot(SlotIndex, FVector::ZeroVector);                                                                
	}       
	
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
	
	// 소환형 슬롯이 GA 실행 중이면 강제 종료 (프리뷰 오브젝트 제거 포함)                                        
	if (Slots[ActiveSlotIndex].SlotEquipData.MoveType == EMoveType::SUMMON)                                      
	{                                                                                                            
		ASC->CancelAbilityHandle(Slots[ActiveSlotIndex].AbilitySpecHandle);                                      
	} 
	
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
	switch (Slot.SlotEquipData.SkillType)
	{
	case ESkillType::ATTACK:
		{
			return (Slot.SlotEquipData.MoveType == EMoveType::SUMMON)
			? RSTags::Event_Weapon_Fire_Summon 
			: RSTags::Event_Weapon_Fire_Projectile;
		}
		
	case ESkillType::DEFENSE:
		return RSTags::Event_Weapon_Defense;
	default:
		{
			KHS_WARN(TEXT("GetEventTag — 매핑 없는 SkillType. WeaponID: %s"),  *Slot.SlotEquipData.WeaponID.ToString());
			return FGameplayTag::EmptyTag;
		}
	}
}

bool UEquipmentSubsystem::LoadEquipData(const FName& WeaponID, FWeaponSlotEquipData& OutData) const
{
	GET_GI_SUBSYSTEM(UGameDataSubsystem, GDS);
	
	if (!GDS->GetWeaponSlotEquipData(WeaponID, OutData))
	{
		KHS_WARN(TEXT("WeaponID 조회 실패: %s"), *WeaponID.ToString());
		return false;
	}
	
	return true;
}

bool UEquipmentSubsystem::LoadEquipClasses(const FWeaponSlotEquipData& EquipData, FLoadedEquipClasses& OutClasses) const
{
	if (!LoadRequiredClass(EquipData.GAClass, OutClasses.GAClass, EquipData.WeaponID))
	{
		return false;
	}
	
	return true;
}

bool UEquipmentSubsystem::RegisterAbility(const FWeaponSlotEquipData& EquipData, const FLoadedEquipClasses& Classes, FGameplayAbilitySpecHandle& OutHandle)
{
	URSSkillData* SkillDataObj = NewObject<URSSkillData>(this);
	SkillDataObj->SkillID  = EquipData.SkillID;
	SkillDataObj->WeaponID = EquipData.WeaponID;
	SkillDataObjects.Add(SkillDataObj);

	FGameplayAbilitySpec Spec(Classes.GAClass, 1, INDEX_NONE, SkillDataObj);
	OutHandle = ASC->GiveAbility(Spec);
	
	if (!ensureMsgf(OutHandle.IsValid(), TEXT("GiveAbility 실패: %s"), *EquipData.WeaponID.ToString()))
	{
		return false;
	}
	
	return true;
}

void UEquipmentSubsystem::CommitSlot(int32 TargetSlot, const FWeaponSlotEquipData& EquipData, const FGameplayAbilitySpecHandle& Handle)
{
	//슬롯 런타임 데이터 관리(GA랑 슬롯은 서로 모르니까)
	FWeaponSlotInstanceData& Slot  = Slots[TargetSlot];
	Slot.SlotEquipData		 = EquipData;
	Slot.AbilitySpecHandle   = Handle;
	Slot.CooldownRemaining   = 0.f;
	Slot.bIsActive           = false;

	KHS_INFO(TEXT("무기 장착 완료: %s → Slot %d"), *EquipData.WeaponID.ToString(), TargetSlot);

	StartAutoFire(TargetSlot);
	//이벤트 발행
	OnSlotUpdatedDel.Broadcast(TargetSlot);
}

void UEquipmentSubsystem::ClearSlot(int32 SlotIndex)
{
	if (!IsValidSlotIndex(SlotIndex))
	{
		KHS_WARN(TEXT("ClearSlot — 유효하지 않은 슬롯 인덱스: %d"), SlotIndex);
		return;
	}

	if (Slots[SlotIndex].IsEmpty())
	{
		return;
	}

	FWeaponSlotInstanceData& Slot = Slots[SlotIndex];

	// 액티브 슬롯이면 비활성화 처리 (StartAutoFire는 하지 않음 — 슬롯을 비울 것이므로)
	if (ActiveSlotIndex == SlotIndex)
	{
		Slot.bIsActive = false;
		if (Slot.SlotEquipData.MoveType == EMoveType::SUMMON)
		{
			ASC->CancelAbilityHandle(Slot.AbilitySpecHandle);
		}
		ActiveSlotIndex = -1;
	}

	StopAutoFire(SlotIndex);
	ASC->ClearAbility(Slot.AbilitySpecHandle);

	// 슬롯 초기화 (SlotIndex 보존)
	Slot = FWeaponSlotInstanceData();
	Slot.SlotIndex = SlotIndex;

	KHS_INFO(TEXT("Slot %d 클리어 완료"), SlotIndex);
	OnSlotUpdatedDel.Broadcast(SlotIndex);
}

void UEquipmentSubsystem::UpgradeWeapon(int32 SlotIndex, FName NextWeaponID)
{
	if (!IsValidSlotIndex(SlotIndex))
	{
		KHS_WARN(TEXT("UpgradeWeapon — 유효하지 않은 슬롯 인덱스: %d"), SlotIndex);
		return;
	}

	KHS_INFO(TEXT("무기 강화: Slot %d → %s"), SlotIndex, *NextWeaponID.ToString());

	ClearSlot(SlotIndex);

	FWeaponSlotEquipData EquipData;
	if (!LoadEquipData(NextWeaponID, EquipData))
	{
		return;
	}

	FLoadedEquipClasses Classes;
	if (!LoadEquipClasses(EquipData, Classes))
	{
		return;
	}

	FGameplayAbilitySpecHandle Handle;
	if (!RegisterAbility(EquipData, Classes, Handle))
	{
		return;
	}

	CommitSlot(SlotIndex, EquipData, Handle);
	PendingWeaponID = NAME_None;
}
