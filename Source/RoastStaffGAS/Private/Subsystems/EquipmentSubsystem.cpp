// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystems/EquipmentSubsystem.h"
#include "RoastStaffGAS.h"
#include "Subsystems/GameDataSubsystem.h"
#include "Subsystems/PoolingSubsystem.h"
#include "Data/RuntimeDataStructs.h"
#include "GAS/Tags/RSGameplayTags.h"
#include "Objects/Data/RSSkillData.h"
#include "System/LoggingSystem.h"
#include "Objects/Projectile/BaseProjectile.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Core/RSGameMode.h"
#include "Character/Enemy/EnemyBaseCharacter.h"
#include "Kismet/GameplayStatics.h"

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
		Slots[i] = FWeaponSlotInstanceData();
		Slots[i].SlotIndex = i;
	}

	bIsInitialized = true;
	KHS_INFO(TEXT("EquipmentSubsystem 초기화 완료"));
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

void UEquipmentSubsystem::DeinitializeSubsystem()
{
	StopAllFire();
	ASC             = nullptr;
	bIsInitialized  = false;
	KHS_INFO(TEXT("EquipmentSubsystem 해제 완료."));
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

void UEquipmentSubsystem::OnSummonAbilityEnded(FGameplayAbilitySpecHandle SpecHandle)
{
	for (int32 i = 0; i < SLOT_COUNT; ++i)
	{
		if (Slots[i].AbilitySpecHandle != SpecHandle)
		{
			continue;
		}

		// 쿨타임 후 최근접 적 위치로 재발동
		const float Cooldown = FMath::Max(0.1f, Slots[i].SlotEquipData.Cooldown);
		GetGameInstance()->GetWorld()->GetTimerManager().SetTimer(
			Slots[i].AutoFireTimerHandle,
			[this, i]()
			{
				AActor* Target = FindNearestEnemy(AutoFireSearchRadius);
				if (!Target)
				{
					return;
				}

				FireSlot(i, Target->GetActorLocation());
				if (Slots[i].SlotEquipData.MoveType == EMoveType::SUMMON && ASC)
				{
					ASC->LocalInputConfirm();
				}
			},
			Cooldown, true, Cooldown);
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
	if (!bIsInitialized || !IsValid(ASC))
	{
		return;
	}

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
	// PreWarm 중에는 자동발사 타이머 등록 스킵
	UWorld* World = GetGameInstance()->GetWorld();
	ARSGameMode* GameMode = World ? World->GetAuthGameMode<ARSGameMode>() : nullptr;
	if (GameMode)
	{
		if (GameMode->bIsPreWarmActive)
		{
			KHS_INFO(TEXT("Slot %d: PreWarm 중 — 자동공격 타이머 등록 스킵"), SlotIndex);
			return;
		}
	}

	FWeaponSlotInstanceData& Slot = Slots[SlotIndex];

	GetGameInstance()->GetWorld()->GetTimerManager().SetTimer(
		Slot.AutoFireTimerHandle,
		[this, SlotIndex]()
		{
			AActor* Target = FindNearestEnemy(AutoFireSearchRadius);
			if (!Target)
			{
				return;
			}

			const FVector TargetLoc = Target->GetActorLocation();
			FireSlot(SlotIndex, TargetLoc);

			// SUMMON 타입: 최근접 적 위치에 즉시 자동 확정
			if (Slots[SlotIndex].SlotEquipData.MoveType == EMoveType::SUMMON && ASC)
			{
				ASC->LocalInputConfirm();
			}
		},
		Slot.SlotEquipData.Cooldown, true, AUTO_FIRE_START_DELAY);  //프리웜&에너미 스폰 시간 고려 지연시간.

	KHS_INFO(TEXT("Slot %d: 자동공격 타이머 시작. 첫 발사: %f초 후, CD: %.2fs"),
		SlotIndex, AUTO_FIRE_START_DELAY, Slot.SlotEquipData.Cooldown);
}

void UEquipmentSubsystem::StopAutoFire(int32 SlotIndex)
{
	GetGameInstance()->GetWorld()->GetTimerManager().ClearTimer(Slots[SlotIndex].AutoFireTimerHandle);
}

AActor* UEquipmentSubsystem::FindNearestEnemy(float SearchRadius) const
{
	if (!ASC)
	{
		return nullptr;
	}

	AActor* AvatarActor = ASC->GetAvatarActor();
	if (!AvatarActor)
	{
		return nullptr;
	}

	const FVector CasterLocation = AvatarActor->GetActorLocation();
	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(AvatarActor);

	GetGameInstance()->GetWorld()->OverlapMultiByChannel(
		Overlaps, CasterLocation, FQuat::Identity, ECC_Pawn,
		FCollisionShape::MakeSphere(SearchRadius), QueryParams);

	AActor* NearestEnemy = nullptr;
	float MinDistSq = MAX_FLT;

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* OverlappedActor = Overlap.GetActor();
		if (!OverlappedActor)
		{
			continue;
		}

		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OverlappedActor);
		if (!TargetASC || !TargetASC->HasMatchingGameplayTag(RSTags::Team_Enemy))
		{
			continue;
		}

		const float DistSq = FVector::DistSquared(CasterLocation, OverlappedActor->GetActorLocation());
		if (DistSq < MinDistSq)
		{
			MinDistSq = DistSq;
			NearestEnemy = OverlappedActor;
		}
	}

	return NearestEnemy;
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

	InitWeaponPool(EquipData);
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

	// SUMMON GA가 실행 중이면 강제 종료 (프리뷰 오브젝트 제거)
	if (Slot.SlotEquipData.MoveType == EMoveType::SUMMON)
	{
		ASC->CancelAbilityHandle(Slot.AbilitySpecHandle);
	}

	StopAutoFire(SlotIndex);
	ASC->ClearAbility(Slot.AbilitySpecHandle);

	// 교체되는 무기 풀 버킷 정리 — 슬롯 리셋 전에 호출해야 EquipData 유효
	ClearWeaponPool(Slot.SlotEquipData);

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

void UEquipmentSubsystem::InitWeaponPool(const FWeaponSlotEquipData& EquipData)
{
	UWorld* World = GetGameInstance()->GetWorld();
	check(World);

	UPoolingSubsystem* PoolSys = World->GetSubsystem<UPoolingSubsystem>();
	check(PoolSys);

	GET_GI_SUBSYSTEM(UGameDataSubsystem, GDS);

	FSkillExecutionData ExecData;
	if (!GDS->GetSkillExecutionData(EquipData.SkillID, ExecData, EquipData.WeaponID))
	{
		KHS_WARN(TEXT("SkillExecutionData 조회 실패 — SkillID: %s"), *EquipData.SkillID.ToString());
		return;
	}

	//스킬 종류에 따라 스킬 소환 오브젝트 풀링
	if (!ExecData.ProjectileClass.IsNull())
	{
		if (TSubclassOf<AActor> ProjClass = ExecData.ProjectileClass.LoadSynchronous())
		{
			PoolSys->InitializePool(ProjClass, WeaponProjectilePoolCount);
			KHS_INFO(TEXT("ProjectilePool 추가 — Class: %s, Count: %d"), *ProjClass->GetName(), WeaponProjectilePoolCount);
		}
	}

	if (!ExecData.SummonObjectClass.IsNull())
	{
		if (TSubclassOf<AActor> SummonClass = ExecData.SummonObjectClass.LoadSynchronous())
		{
			PoolSys->InitializePool(SummonClass, WeaponSummonPoolCount);
			KHS_INFO(TEXT("SummonObjectPool 추가 — Class: %s, Count: %d"), *SummonClass->GetName(), WeaponSummonPoolCount);
		}
	}
}

void UEquipmentSubsystem::ClearWeaponPool(const FWeaponSlotEquipData& EquipData)
{
	if (EquipData.SkillID.IsNone())
	{
		return;
	}

	UWorld* World = GetGameInstance()->GetWorld();
	check(World);

	UPoolingSubsystem* PoolSys = World->GetSubsystem<UPoolingSubsystem>();
	check(PoolSys);

	GET_GI_SUBSYSTEM(UGameDataSubsystem, GDS);

	FSkillExecutionData ExecData;
	if (!GDS->GetSkillExecutionData(EquipData.SkillID, ExecData, EquipData.WeaponID))
	{
		return;
	}

	// Get()만 사용 — 이미 메모리에 올라온 클래스만 풀 존재, 없으면 드레인 불필요
	if (TSubclassOf<AActor> ProjClass = ExecData.ProjectileClass.Get())
	{
		PoolSys->DrainPool(ProjClass);
	}

	if (TSubclassOf<AActor> SummonClass = ExecData.SummonObjectClass.Get())
	{
		PoolSys->DrainPool(SummonClass);
	}
}
