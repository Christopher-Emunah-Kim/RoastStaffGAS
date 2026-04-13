// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystems/SkillManagerSubsystem.h"
#include "RoastStaffGAS.h"
#include "GAS/Tags/RSGameplayTags.h"
#include "Subsystems/GameDataSubsystem.h"
#include "Objects/Data/RSCharacterSkillData.h"
#include "Objects/Summon/SummonPreviewObject.h"
#include "Core/RSGameMode.h"
#include "System/LoggingSystem.h"
#include "AbilitySystemComponent.h"

const FCharacterSkillExecData USkillManagerSubsystem::EmptyExecData = FCharacterSkillExecData{};

void USkillManagerSubsystem::InitializeSkills(FName CharacterID, UAbilitySystemComponent* InASC)
{
	if (!ensureMsgf(InASC, TEXT("ASC가 null")))
	{
		return;
	}

	if (CharacterID.IsNone())
	{
		KHS_WARN(TEXT("CharacterID가 NAME_None — 스킬 초기화 건너뜀"));
		return;
	}

	ASC = InASC;
	SkillDataObjects.Reset();

	GET_GI_SUBSYSTEM_FROM(UGameDataSubsystem, GDS, GetWorld()->GetGameInstance())

	// 슬롯 1(Q)=SlotIndex 0, 슬롯 2(E)=SlotIndex 1
	for (int32 i = 0; i < SKILL_SLOT_COUNT; ++i)
	{
		const int32 SkillSlot = i + 1; // DT SkillSlot 필드는 1/2 기반

		FCharacterSkillExecData ExecData;
		if (!GDS->GetCharacterSkillExecData(CharacterID, SkillSlot, 1, ExecData))
		{
			KHS_WARN(TEXT("CharID[%s] Slot[%d] ExecData 조회 실패 — 해당 슬롯 스킬 없음"), *CharacterID.ToString(), SkillSlot);
			continue;
		}

		if (ExecData.GAClass.IsNull())
		{
			KHS_WARN(TEXT("CharID[%s] Slot[%d] GAClass 미설정 — 스킵"), *CharacterID.ToString(), SkillSlot);
			continue;
		}

		TSubclassOf<UGameplayAbility> GAClass = ExecData.GAClass.LoadSynchronous();
		if (!GAClass)
		{
			KHS_WARN(TEXT("CharID[%s] Slot[%d] GAClass 로드 실패"), *CharacterID.ToString(), SkillSlot);
			continue;
		}

		// SourceObject 생성 — GA가 SlotIndex로 ExecData 역참조
		URSCharacterSkillData* DataObj = NewObject<URSCharacterSkillData>(this);
		DataObj->SlotIndex = i;
		DataObj->SkillID   = ExecData.SkillID;
		SkillDataObjects.Add(DataObj);

		FGameplayAbilitySpec Spec(GAClass, 1, INDEX_NONE, DataObj);
		FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(Spec);

		if (!ensureMsgf(Handle.IsValid(), TEXT("GiveAbility 실패 — CharID: %s Slot: %d"), *CharacterID.ToString(), SkillSlot))
		{
			continue;
		}

		SkillSlots[i].ExecData = ExecData;
		SkillSlots[i].Handle   = Handle;

		KHS_INFO(TEXT("스킬 등록 완료 — CharID: %s | Slot %d | SkillID: %s"),
			*CharacterID.ToString(), SkillSlot, *ExecData.SkillID.ToString());
	}

	bIsInitialized = true;
	KHS_INFO(TEXT("SkillManagerSubsystem 초기화 완료 — CharID: %s"), *CharacterID.ToString());
}

void USkillManagerSubsystem::ActivateSkillSlot(int32 SlotIndex)
{
	if (!bIsInitialized || !ASC)
	{
		KHS_WARN(TEXT("초기화 전 호출"));
		return;
	}

	if (SlotIndex < 0 || SlotIndex >= SKILL_SLOT_COUNT)
	{
		KHS_WARN(TEXT("유효하지 않은 SlotIndex %d"), SlotIndex);
		return;
	}

	FSkillSlotState& Slot = SkillSlots[SlotIndex];

	if (!Slot.Handle.IsValid())
	{
		KHS_WARN(TEXT("Slot %d — GA 핸들 미등록"), SlotIndex);
		return;
	}

	if (Slot.bIsOnCooldown)
	{
		KHS_INFO(TEXT("Slot %d 쿨타임 중"), SlotIndex);
		return;
	}

	if (IsPreviewActive())
	{
		KHS_INFO(TEXT("프리뷰 활성 중 — 중복 발동 불가"));
		return;
	}

	const ESkillActivationType Type = Slot.ExecData.ActivationType;

	if (Type == ESkillActivationType::SpawnPreview)
	{
		SpawnPreviewActor(SlotIndex);
	}
	else
	{
		// InstantAoE / SelfBuff — GA 즉발
		ASC->TryActivateAbility(Slot.Handle, true);
		StartCooldown(SlotIndex);
		
		KHS_INFO(TEXT("Slot %d 즉발 — SkillID: %s"), SlotIndex, *Slot.ExecData.SkillID.ToString());
	}
}

void USkillManagerSubsystem::ConfirmSkillPreview(FVector WorldLocation)
{
	if (!IsPreviewActive())
	{
		return;
	}

	PendingTargetLocation = WorldLocation;
	DestroyPreviewActor();

	// Preview.Active 태그 제거 (프리뷰 상태 해제)
	if (ASC)
	{
		ASC->RemoveLooseGameplayTag(RSTags::Skill_Character_Preview_Active);
	}

	// GA 발동 — PendingTargetLocation을 GA가 읽음
	const int32 Slot = ActivePreviewSlot;
	ActivePreviewSlot = -1;

	if (SkillSlots[Slot].Handle.IsValid())
	{
		ASC->TryActivateAbility(SkillSlots[Slot].Handle, true);
		StartCooldown(Slot);

		KHS_INFO(TEXT("Slot %d 확정 — 위치: %s"), Slot, *WorldLocation.ToString());
	}
}

void USkillManagerSubsystem::CancelSkillPreview()
{
	if (!IsPreviewActive())
	{
		return;
	}

	DestroyPreviewActor();

	if (ASC)
	{
		ASC->RemoveLooseGameplayTag(RSTags::Skill_Character_Preview_Active);
	}

	KHS_INFO(TEXT("CancelSkillPreview: Slot %d 취소 — 쿨타임 없음"), ActivePreviewSlot);
	ActivePreviewSlot = -1;
}

const FCharacterSkillExecData& USkillManagerSubsystem::GetSlotExecData(int32 SlotIndex) const
{
	if (SlotIndex < 0 || SlotIndex >= SKILL_SLOT_COUNT)
	{
		KHS_WARN(TEXT("GetSlotExecData: 범위 초과 SlotIndex %d"), SlotIndex);
		return EmptyExecData;
	}

	return SkillSlots[SlotIndex].ExecData;
}

void USkillManagerSubsystem::SpawnPreviewActor(int32 SlotIndex)
{
	UWorld* World = GetWorld();
	if (!World || !ASC)
	{
		return;
	}

	AActor* AvatarActor = ASC->GetAvatarActor();
	if (!AvatarActor)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ARSGameMode* GM = Cast<ARSGameMode>(World->GetAuthGameMode());
	TSubclassOf<ASummonPreviewObject> SpawnClass = nullptr;
	
	if (GM && GM->GetPreviewActorClass())
	{
		SpawnClass = GM->GetPreviewActorClass();
	}
	else
	{
		SpawnClass = ASummonPreviewObject::StaticClass();
	}
	
	ASummonPreviewObject* PreviewActor = World->SpawnActor<ASummonPreviewObject>(SpawnClass, AvatarActor->GetActorLocation(),
		FRotator::ZeroRotator, SpawnParams);

	if (!PreviewActor)
	{
		KHS_WARN(TEXT("스폰 실패 — Slot %d"), SlotIndex);
		return;
	}

	ActivePreviewActor = PreviewActor;
	ActivePreviewSlot  = SlotIndex;

	ASC->AddLooseGameplayTag(RSTags::Skill_Character_Preview_Active);
	
	KHS_INFO(TEXT("SpawnPreviewActor: 프리뷰 활성 — Slot %d | SkillID: %s"),
		SlotIndex, *SkillSlots[SlotIndex].ExecData.SkillID.ToString());
}

void USkillManagerSubsystem::DestroyPreviewActor()
{
	if (ActivePreviewActor)
	{
		ActivePreviewActor->Destroy();
		ActivePreviewActor = nullptr;
	}
}

void USkillManagerSubsystem::StartCooldown(int32 SlotIndex)
{
	const float Cooldown = FMath::Max(0.1f, SkillSlots[SlotIndex].ExecData.Cooldown);
	SkillSlots[SlotIndex].bIsOnCooldown = true;

	GetWorld()->GetTimerManager().SetTimer(	SkillSlots[SlotIndex].CooldownTimer,
		[this, SlotIndex]()
		{
			SkillSlots[SlotIndex].bIsOnCooldown = false;
			KHS_INFO(TEXT("Skill Slot %d 쿨타임 종료"), SlotIndex);
		},
		Cooldown, false);

	KHS_INFO(TEXT("Slot %d 쿨타임 시작 — %.1fs"), SlotIndex, Cooldown);
}
