// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystems/PassiveSlotSubsystem.h"
#include "RoastStaffGAS.h"
#include "GAS/Tags/RSGameplayTags.h"
#include "Subsystems/GameDataSubsystem.h"
#include "Data/DataTableStructs.h"
#include "System/LoggingSystem.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"

void UPassiveSlotSubsystem::InitializeSubsystem(UAbilitySystemComponent* InASC)
{
	if (!ensureMsgf(InASC, TEXT("ASC가 null")))
	{
		return;
	}

	ASC = InASC;
	bIsInitialized = true;
	KHS_INFO(TEXT("PassiveSlotSubsystem 초기화 완료"));
}

bool UPassiveSlotSubsystem::TryAddPassive(FName PassiveID)
{
	if (!ensureMsgf(bIsInitialized && ASC, TEXT("TryAddPassive 호출 전 InitializeSubsystem 필요")))
	{
		return false;
	}

	if (IsSlotFull())
	{
		KHS_WARN(TEXT("패시브 슬롯 포화 — PassiveID: %s 추가 불가"), *PassiveID.ToString());
		return false;
	}

	GET_GI_SUBSYSTEM_FROM(UGameDataSubsystem, GDS, GetWorld()->GetGameInstance())

	FPassiveStaticData PassiveData;
	if (!GDS->GetPassiveData(PassiveID, PassiveData))
	{
		KHS_WARN(TEXT("패시브 데이터 조회 실패 — PassiveID: %s"), *PassiveID.ToString());
		return false;
	}

	if (PassiveData.GEClass.IsNull())
	{
		KHS_WARN(TEXT("패시브 GEClass 미설정 — PassiveID: %s"), *PassiveID.ToString());
		return false;
	}

	TSubclassOf<UGameplayEffect> GEClass = PassiveData.GEClass.LoadSynchronous();
	if (!GEClass)
	{
		KHS_WARN(TEXT("패시브 GEClass 로드 실패 — PassiveID: %s"), *PassiveID.ToString());
		return false;
	}

	// 영구 GE 적용 (Duration = Infinite → BP에서 GE 설정 필요)
	FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
	ContextHandle.AddSourceObject(this);
	FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(GEClass, 1, ContextHandle);

	if (!Spec.IsValid())
	{
		KHS_WARN(TEXT("GE 스펙 생성 실패 — PassiveID: %s"), *PassiveID.ToString());
		return false;
	}

	// DT_Passive.Magnitude → GE SetByCaller 주입 (GE Modifier가 Data.PassiveMagnitude 태그 사용)
	Spec.Data->SetByCallerTagMagnitudes.Add(RSTags::Data_PassiveMagnitude, PassiveData.Magnitude);

	ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
	EquippedPassiveIDs.Add(PassiveID);

	KHS_INFO(TEXT("패시브 장착 완료 — PassiveID: %s | 슬롯: %d/%d"), *PassiveID.ToString(), EquippedPassiveIDs.Num(), MAX_SLOTS);

	// 슬롯 포화 태그 부여
	if (IsSlotFull())
	{
		ASC->AddLooseGameplayTag(RSTags::Passive_SlotFull);
		KHS_INFO(TEXT("패시브 슬롯 포화 — Passive.SlotFull 태그 부여"));
	}

	OnPassiveSlotChangedDel.Broadcast();
	return true;
}
