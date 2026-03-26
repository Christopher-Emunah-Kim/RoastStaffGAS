// Fill out your copyright notice in the Description page of Project Settings.

#include "Subsystems/LevelUpSubsystem.h"
#include "RoastStaffGAS.h"
#include "GAS/Attributes/PlayerAttributeSet.h"
#include "Subsystems/GameDataSubsystem.h"
#include "Subsystems/EquipmentSubsystem.h"
#include "GAS/Tags/RSGameplayTags.h"
#include "Data/DataTableStructs.h"
#include "Algo/RandomShuffle.h"

void ULevelUpSubsystem::InitializeSubsystem(
	UAbilitySystemComponent* InASC,
	UPlayerAttributeSet* InAttributeSet,
	TSubclassOf<UGameplayEffect> InAddEXPEffectClass)
{
	if (bIsInitialized)
	{
		KHS_WARN(TEXT("LevelUpSubsystem: 이미 초기화됨. 중복 호출 무시."));
		return;
	}

	if (!InASC || !InAttributeSet || !InAddEXPEffectClass)
	{
		KHS_WARN(TEXT("필수 초기화 데이터 누락"));
		return;
	}

	ASC             = InASC;
	AttributeSet    = InAttributeSet;
	AddEXPEffectClass = InAddEXPEffectClass;

	AttributeSet->OnEXPChangedDel.AddDynamic(this, &ULevelUpSubsystem::OnEXPChanged);

	bIsInitialized = true;
	KHS_INFO(TEXT("LevelUpSubsystem 초기화 완료"));
}

void ULevelUpSubsystem::AddEXP(float Amount)
{
	if (!ensureMsgf(ASC, TEXT("ASC가 null")))
	{
		return;
	}

	if (Amount <= 0.f)
	{
		KHS_WARN(TEXT("Amount가 0 이하 — 무시"));
		return;
	}

	const int32 CurrentLevel = static_cast<int32>(AttributeSet->GetLevel());
	if (CurrentLevel >= MAX_LEVEL)
	{
		KHS_INFO(TEXT("최대 레벨 도달 — EXP 누적 중단"));
		return;
	}

	if (!ensureMsgf(AddEXPEffectClass, TEXT("AddEXPEffectClass가 설정되지 않음")))
	{
		return;
	}

	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(AddEXPEffectClass, 1.f, Context);

	if (!Spec.IsValid())
	{
		KHS_WARN(TEXT("GE_AddEXP Spec 로드 실패"));
		return;
	}

	Spec.Data->SetByCallerTagMagnitudes.Add(RSTags::Data_EXP, Amount);
	ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());

	KHS_INFO(TEXT("EXP GE 적용 : +%.0f"), Amount);
}

void ULevelUpSubsystem::OnEXPChanged(float NewEXP, int32 CurrentLevel)
{
	CheckLevelUp(NewEXP, CurrentLevel);
}

void ULevelUpSubsystem::CheckLevelUp(float NewEXP, int32 CurrentLevel)
{
	if (bIsLevelingUp || CurrentLevel >= MAX_LEVEL)
	{
		return;
	}

	GET_GI_SUBSYSTEM(UGameDataSubsystem, GDS);

	float RequiredExp = 0.f;
	if (!GDS->GetLevelCurveValue(FName("RequiredEXP"), CurrentLevel + 1, RequiredExp))
	{
		return;
	}

	if (NewEXP < RequiredExp)
	{
		return;
	}

	const float OverflowEXP = NewEXP - RequiredExp;
	bIsLevelingUp = true;

	KHS_INFO(TEXT("레벨업! %d → %d (남은EXP: %.0f)"), CurrentLevel, CurrentLevel + 1, OverflowEXP);

	ApplyLevelUp(CurrentLevel, OverflowEXP);
	SelectWeaponCandidates();
	// bIsLevelingUp는 PlayerController가 UI 종료 후 NotifyWeaponSelectCompleted()로 해제
	// 잉여 EXP 연속 레벨업은 Task B에서 처리
}

void ULevelUpSubsystem::SelectWeaponCandidates()
{
	GET_GI_SUBSYSTEM(UGameDataSubsystem, GDS);
	GET_GI_SUBSYSTEM(UEquipmentSubsystem, EquipSys);

	TArray<FName> WeaponPool = GDS->GetWeaponIDsByLevel(1);

	if (WeaponPool.IsEmpty())
	{
		KHS_WARN(TEXT("무기 풀이 비어있음 — 레벨업 무기 선정 실패"));
		check(false);
		return;
	}

	Algo::RandomShuffle(WeaponPool);

	const int32 PickCount = FMath::Min(WeaponPool.Num(), WEAPON_CANDIDATE_COUNT);
	TArray<FWeaponCardDisplayData> Candidates;

	for (int32 i = 0; i < PickCount; i++)
	{
		const FName& CandidateID = WeaponPool[i];

		FWeaponStaticData CandidateData;
		if (!GDS->GetWeaponData(CandidateID, CandidateData))
		{
			KHS_WARN(TEXT("후보 무기 데이터 조회 실패 — ID: %s, 건너뜀"), *CandidateID.ToString());
			continue;
		}

		// 현재 장착 슬롯과 BaseType 비교로 카드 상태 결정
		EWeaponCardState CardState = EWeaponCardState::New;
		for (int32 SlotIdx = 0; SlotIdx < EquipSys->GetSlotCount(); SlotIdx++)
		{
			const FWeaponSlotInstanceData* SlotData = EquipSys->GetSlotData(SlotIdx);
			if (!SlotData || SlotData->IsEmpty())
			{
				continue;
			}

			FWeaponStaticData EquippedData;
			if (!GDS->GetWeaponData(SlotData->SlotEquipData.WeaponID, EquippedData))
			{
				continue;
			}

			if (EquippedData.BaseType != CandidateData.BaseType)
			{
				continue;
			}

			switch (EquippedData.WeaponLevel)
			{
				case 1:  CardState = EWeaponCardState::Lv1ToLv2; break;
				case 2:  CardState = EWeaponCardState::Lv2ToLv3; break;
				case 3:  CardState = EWeaponCardState::Lv3Max;   break;
				default: break;
			}
			break; // 첫 번째 일치 슬롯 기준
		}

		FWeaponSlotEquipData EquipData;
		FWeaponCardDisplayData CardData;
		CardData.WeaponID    = CandidateID;
		CardData.WeaponName  = CandidateData.WeaponName;
		CardData.Description = CandidateData.Description;
		CardData.CardState   = CardState;
		CardData.bCanEvolve  = false; // DT_Combination 미구현 스텁
		if (GDS->GetWeaponSlotEquipData(CandidateID, EquipData))
		{
			CardData.WeaponIcon = EquipData.SkillIcon;
		}

		Candidates.Add(CardData);
	}

	if (Candidates.IsEmpty())
	{
		KHS_WARN(TEXT("유효 후보 0종 — 델리게이트 발행 취소"));
		bIsLevelingUp = false;
		return;
	}

	KHS_INFO(TEXT("무기 후보 선정 완료 — %d종 발행"), Candidates.Num());
	OnWeaponCandidatesReadyDel.Broadcast(Candidates);
}

void ULevelUpSubsystem::ApplyLevelUp(int32 CurrentLevel, float OverflowEXP)
{
	ASC->SetNumericAttributeBase(AttributeSet->GetLevelAttribute(), static_cast<float>(CurrentLevel + 1));
	ASC->SetNumericAttributeBase(AttributeSet->GetEXPAttribute(), OverflowEXP);

	KHS_INFO(TEXT("어트리뷰트 갱신 — Level: %d, EXP: %.0f"), CurrentLevel + 1, OverflowEXP);
}

void ULevelUpSubsystem::NotifyWeaponSelectCompleted()
{
	bIsLevelingUp = false;
	KHS_INFO(TEXT("무기 선택 완료 알림 수신 — bIsLevelingUp 해제"));
}
