// Fill out your copyright notice in the Description page of Project Settings.

#include "Subsystems/LevelUpSubsystem.h"
#include "RoastStaffGAS.h"
#include "GAS/Attributes/PlayerAttributeSet.h"
#include "GAS/Attributes/BaseAttributeSet.h"
#include "Subsystems/GameDataSubsystem.h"
#include "Subsystems/EquipmentSubsystem.h"
#include "Subsystems/PassiveSlotSubsystem.h"
#include "GAS/Tags/RSGameplayTags.h"
#include "Data/DataTableStructs.h"
#include "Algo/RandomShuffle.h"

namespace
{
	FGameplayAttribute ResolveStatAttribute(FName StatType)
	{
		if (StatType == "ATK")            return UPlayerAttributeSet::GetATKAttribute();
		if (StatType == "DEF")            return UPlayerAttributeSet::GetDEFAttribute();
		if (StatType == "MaxHP")          return UBaseAttributeSet::GetMaxHPAttribute();
		if (StatType == "CriticalRate")   return UPlayerAttributeSet::GetCriticalRateAttribute();
		if (StatType == "CriticalDamage") return UPlayerAttributeSet::GetCriticalDamageAttribute();
		if (StatType == "AttackSpeed")    return UPlayerAttributeSet::GetAttackSpeedAttribute();
		return FGameplayAttribute{};
	}
}

void ULevelUpSubsystem::InitializeSubsystem(UAbilitySystemComponent* InASC,	UPlayerAttributeSet* InAttributeSet,TSubclassOf<UGameplayEffect> InAddEXPEffectClass)
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

void ULevelUpSubsystem::OnEnemyKilled(FName InEnemyID)
{
	if (!bIsInitialized)
	{
		KHS_WARN(TEXT("LevelUpSubsystem::OnEnemyKilled — 미초기화 상태. ASC 연결 전 호출됨"));
		return;
	}

	GET_GI_SUBSYSTEM(UGameDataSubsystem, GDS)
	FEnemyStaticData EnemyData;
	if (!GDS->GetEnemyData(InEnemyID, EnemyData))
	{
		KHS_WARN(TEXT("LevelUpSubsystem::OnEnemyKilled — EnemyID 조회 실패: %s"), *InEnemyID.ToString());
		return;
	}

	if (EnemyData.DropEXP <= 0)
	{
		return;
	}

	AddEXP(static_cast<float>(EnemyData.DropEXP));
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

	GET_GI_SUBSYSTEM(UGameDataSubsystem, GDS)

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
	BuildCardPool();
	// bIsLevelingUp는 PlayerController가 UI 종료 후 NotifyWeaponSelectCompleted()로 해제
}

// ============================================================================
// 카드풀 구성
// ============================================================================

void ULevelUpSubsystem::BuildCardPool()
{
	TArray<FLevelUpCardDisplayData> AllCards = BuildStaticCardPool();
	AllCards.Append(BuildDynamicWeaponCards());
	EnsureWeaponCardGuarantee(AllCards);

	TArray<FLevelUpCardDisplayData> FinalCards = PickFinalCards(AllCards);

	if (FinalCards.IsEmpty())
	{
		KHS_WARN(TEXT("카드풀이 비어있음 — 레벨업 UI 취소"));
		bIsLevelingUp = false;
		return;
	}

	KHS_INFO(TEXT("카드풀 선정 완료 — %d장 발행"), FinalCards.Num());
	OnCardPoolReadyDel.Broadcast(FinalCards);
}

TArray<FLevelUpCardDisplayData> ULevelUpSubsystem::BuildStaticCardPool()
{
	GET_GI_SUBSYSTEM(UGameDataSubsystem, GDS)
	GET_WORLD_SUBSYSTEM(UPassiveSlotSubsystem, PassiveSys)
	
	const bool bPassiveFull = PassiveSys && PassiveSys->IsSlotFull();

	TArray<FLevelUpCardDisplayData> Pool;

	for (const FLevelUpCardStaticData& CardData : GDS->GetAllLevelUpCards())
	{
		//패시브 슬롯이 full이면 패시브 카드는 출현하지 않음.
		if (CardData.CardType == ELevelUpCardType::PassiveAdd && bPassiveFull)
		{
			continue;
		}

		FLevelUpCardDisplayData Card;
		Card.CardID      = CardData.CardID;
		Card.CardType    = CardData.CardType;
		Card.DisplayName = CardData.DisplayName;
		Card.Description = CardData.Description;
		Card.Weight      = CardData.Weight;
		Card.Icon        = CardData.Icon; // StatUpgrade: DT_LevelUpCard.Icon 직접 사용

		if (CardData.CardType == ELevelUpCardType::PassiveAdd && !CardData.PassiveID.IsNone())
		{
			FPassiveStaticData PassiveData;
			if (GDS->GetPassiveData(CardData.PassiveID, PassiveData))
			{
				Card.Icon = PassiveData.Icon;
				if (!PassiveData.Description.IsEmpty())
				{
					Card.Description = PassiveData.Description;
				}
			}
		}

		Pool.Add(Card);
	}

	return Pool;
}

TArray<FLevelUpCardDisplayData> ULevelUpSubsystem::BuildDynamicWeaponCards()
{
	GET_GI_SUBSYSTEM(UGameDataSubsystem, GDS)
	GET_GI_SUBSYSTEM(UEquipmentSubsystem, EquipSys)

	TSet<EWeaponBaseType> EquippedBaseTypes;
	TArray<FLevelUpCardDisplayData> DynamicCards;

	// 1. 장착 무기 업그레이드 카드 생성 (같은 BaseType + 다음 레벨)
	for (int32 i = 0; i < EquipSys->GetSlotCount(); i++)
	{
		const FWeaponSlotInstanceData* SlotData = EquipSys->GetSlotData(i);
		if (!SlotData || SlotData->IsEmpty())
		{
			continue;
		}

		FWeaponStaticData EquippedData;
		if (!GDS->GetWeaponData(SlotData->SlotEquipData.WeaponID, EquippedData))
		{
			continue;
		}

		EquippedBaseTypes.Add(EquippedData.BaseType);

		if (EquippedData.WeaponLevel >= 3)
		{
			continue; // MAX 레벨 — 업그레이드 불가
		}

		TArray<FName> NextLevelWeapons = GDS->GetWeaponIDsByLevel(EquippedData.WeaponLevel + 1);
		for (const FName& NextWeaponID : NextLevelWeapons)
		{
			FWeaponStaticData NextData;
			if (!GDS->GetWeaponData(NextWeaponID, NextData))
			{
				continue;
			}

			if (NextData.BaseType != EquippedData.BaseType)
			{
				continue;
			}

			FLevelUpCardDisplayData Card;
			Card.CardID      = NextWeaponID;
			Card.CardType    = ELevelUpCardType::WeaponUpgrade;
			Card.DisplayName = FText::FromName(NextData.WeaponName);
			Card.Description = FText::FromName(NextData.Description);
			Card.Weight      = 1.5f;

			FWeaponSlotEquipData EquipData;
			if (GDS->GetWeaponSlotEquipData(NextWeaponID, EquipData))
			{
				Card.Icon = EquipData.SkillIcon;
			}

			DynamicCards.Add(Card);
			break; // 슬롯당 업그레이드 카드 1장
		}
	}

	// 2. 미장착 BaseType의 신규 무기 카드 생성 (Lv1 기준)
	for (const FName& WeaponID : GDS->GetWeaponIDsByLevel(1))
	{
		FWeaponStaticData WData;
		if (!GDS->GetWeaponData(WeaponID, WData))
		{
			continue;
		}

		if (EquippedBaseTypes.Contains(WData.BaseType))
		{
			continue; // 이미 장착한 계열 — WeaponUpgrade로 처리됨
		}

		FLevelUpCardDisplayData Card;
		Card.CardID      = WeaponID;
		Card.CardType    = ELevelUpCardType::WeaponNew;
		Card.DisplayName = FText::FromName(WData.WeaponName);
		Card.Description = FText::FromName(WData.Description);
		Card.Weight      = 1.0f;

		FWeaponSlotEquipData EquipData;
		if (GDS->GetWeaponSlotEquipData(WeaponID, EquipData))
		{
			Card.Icon = EquipData.SkillIcon;
		}

		DynamicCards.Add(Card);
	}

	return DynamicCards;
}

void ULevelUpSubsystem::EnsureWeaponCardGuarantee(TArray<FLevelUpCardDisplayData>& CardPool)
{
	const bool bHasWeaponCard = CardPool.ContainsByPredicate([](const FLevelUpCardDisplayData& Card)
	{
		return Card.CardType == ELevelUpCardType::WeaponUpgrade
			|| Card.CardType == ELevelUpCardType::WeaponNew;
	});

	if (bHasWeaponCard)
	{
		return;
	}

	TArray<FLevelUpCardDisplayData> WeaponCards = BuildDynamicWeaponCards();
	if (!WeaponCards.IsEmpty())
	{
		CardPool.Add(WeaponCards[0]);
		KHS_INFO(TEXT("무기 카드 강제 추가 — CardID: %s"), *WeaponCards[0].CardID.ToString());
	}
	else
	{
		KHS_WARN(TEXT("무기 카드 보장 불가 — 무기 풀 비어있음"));
	}
}

TArray<FLevelUpCardDisplayData> ULevelUpSubsystem::PickFinalCards(const TArray<FLevelUpCardDisplayData>& Pool)
{
	if (Pool.IsEmpty())
	{
		return {};
	}

	TArray<FLevelUpCardDisplayData> Remaining = Pool;
	TArray<FLevelUpCardDisplayData> Result;

	while (Result.Num() < CARD_PICK_COUNT && !Remaining.IsEmpty())
	{
		float TotalWeight = 0.f;
		for (const FLevelUpCardDisplayData& Card : Remaining)
		{
			TotalWeight += Card.Weight;
		}

		const float Roll = FMath::FRandRange(0.f, TotalWeight);
		float Accumulated = 0.f;
		int32 PickedIndex = 0;

		for (int32 i = 0; i < Remaining.Num(); i++)
		{
			Accumulated += Remaining[i].Weight;
			if (Roll <= Accumulated)
			{
				PickedIndex = i;
				break;
			}
		}

		Result.Add(Remaining[PickedIndex]);
		Remaining.RemoveAtSwap(PickedIndex);
	}

	return Result;
}

// ============================================================================
// 카드 선택 처리
// ============================================================================

void ULevelUpSubsystem::OnCardSelected(FName CardID)
{
	GET_GI_SUBSYSTEM(UGameDataSubsystem, GDS)

	FLevelUpCardStaticData CardData;
	if (GDS->GetLevelUpCardData(CardID, CardData))
	{
		switch (CardData.CardType)
		{
			case ELevelUpCardType::StatUpgrade:
			{
					ApplyStatUpgrade(CardData);
					return;
			}
			case ELevelUpCardType::PassiveAdd:
			{
				GET_WORLD_SUBSYSTEM(UPassiveSlotSubsystem, PassiveSys)
				PassiveSys->TryAddPassive(CardData.PassiveID);
				return;
			}
			
			default:
			{
				return;
			}
		}
	}

	// 정적 카드 미발견 → 동적 무기 카드 (WeaponUpgrade / WeaponNew)
	GET_GI_SUBSYSTEM(UEquipmentSubsystem, EquipSys)
	EquipSys->EquipWeapon(CardID);
	KHS_INFO(TEXT("무기 장착 요청 — CardID: %s"), *CardID.ToString());
}

void ULevelUpSubsystem::ApplyStatUpgrade(const FLevelUpCardStaticData& CardData)
{
	const FGameplayAttribute Attr = ResolveStatAttribute(CardData.StatType);
	if (!Attr.IsValid())
	{
		KHS_WARN(TEXT("알 수 없는 StatType — %s"), *CardData.StatType.ToString());
		return;
	}

	const float CurrentBase = ASC->GetNumericAttributeBase(Attr);
	const float NewBase     = CurrentBase + CardData.StatModifier;
	ASC->SetNumericAttributeBase(Attr, NewBase);

	if (CardData.StatType == "MaxHP")
	{
		ASC->SetNumericAttributeBase(UBaseAttributeSet::GetCurrentHPAttribute(), NewBase);
	}

	KHS_INFO(TEXT("스탯 업그레이드 — %s: %.1f → %.1f"), *CardData.StatType.ToString(), CurrentBase, NewBase);
}

// ============================================================================
// 완료 처리
// ============================================================================

void ULevelUpSubsystem::ApplyLevelUp(int32 CurrentLevel, float OverflowEXP)
{
	const int32 NewLevel = CurrentLevel + 1;
	ASC->SetNumericAttributeBase(AttributeSet->GetLevelAttribute(), static_cast<float>(NewLevel));
	ASC->SetNumericAttributeBase(AttributeSet->GetEXPAttribute(), OverflowEXP);

	GET_GI_SUBSYSTEM(UGameDataSubsystem, GDS)
	float NewMaxHP = 0.f;
	if (GDS->GetLevelCurveValue(FName("MaxHP"), NewLevel, NewMaxHP) && NewMaxHP > 0.f)
	{
		ASC->SetNumericAttributeBase(UBaseAttributeSet::GetMaxHPAttribute(), NewMaxHP);
		ASC->SetNumericAttributeBase(UBaseAttributeSet::GetCurrentHPAttribute(), NewMaxHP);
		KHS_INFO(TEXT("MaxHP 갱신 — Level: %d, MaxHP: %.0f"), NewLevel, NewMaxHP);
	}
	else
	{
		KHS_WARN(TEXT("MaxHP 커브 조회 실패 — Level: %d"), NewLevel);
	}

	KHS_INFO(TEXT("어트리뷰트 갱신 — Level: %d, EXP: %.0f"), NewLevel, OverflowEXP);
}

void ULevelUpSubsystem::NotifyWeaponSelectCompleted()
{
	bIsLevelingUp = false;
	KHS_INFO(TEXT("카드 선택 완료 알림 수신 — bIsLevelingUp 해제"));
}
