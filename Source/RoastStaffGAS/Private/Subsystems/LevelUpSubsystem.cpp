// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystems/LevelUpSubsystem.h"
#include "RoastStaffGAS.h"
#include "GAS/Attributes/PlayerAttributeSet.h"
#include "Subsystems/GameDataSubsystem.h"
#include "Subsystems/EquipmentSubsystem.h"
#include "GAS/Tags/RSGameplayTags.h"
#include "Data/DataTableStructs.h"
#include "System/LoggingSystem.h"

void ULevelUpSubsystem::InitializeSubsystem(UAbilitySystemComponent* InASC, UPlayerAttributeSet* InAttributeSet,TSubclassOf<UGameplayEffect> InAddEXPEffectClass)
{
	if (bIsInitialized)
	{
		KHS_WARN(TEXT("LevelUpSubsystem: 이미 초기화됨. 중복 호출 무시."));
		return;
	}

	if (!InASC || !InAttributeSet || !InAddEXPEffectClass )
	{
		KHS_WARN(TEXT("필수 초기화 데이터 누락"));
		return;
	}
	
	ASC				  = InASC;
	AttributeSet	  = InAttributeSet;
	AddEXPEffectClass = InAddEXPEffectClass;

	AttributeSet->OnEXPChangedDel.AddDynamic(this, &ULevelUpSubsystem::OnEXPChanged); //경험치 이벤트 구독

	bIsInitialized = true;
	KHS_INFO(TEXT("LevelUpSubsystem 초기화 완료"));
}

void ULevelUpSubsystem::AddEXP(float Amount)
{
	KHS_INFO(TEXT("AddEXP 호출됨. Amount: %.0f / bIsInitialized: %d / ASC: %d"),
		Amount, bIsInitialized, ASC != nullptr);
	
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

	UGameDataSubsystem* GDS = GetGameInstance()->GetSubsystem<UGameDataSubsystem>();
	check(GDS);

	float CurrentExpToProcess  = NewEXP;
	int32 LevelToProcess       = CurrentLevel;
	float RequiredExp		   = 0.f;
	const bool bSuccessLoadExp = GDS->GetLevelCurveValue(FName("RequiredEXP"), LevelToProcess + 1, RequiredExp);
	
	while (LevelToProcess < MAX_LEVEL && bSuccessLoadExp && CurrentExpToProcess >= RequiredExp)
	{
		CurrentExpToProcess -= RequiredExp;

		bIsLevelingUp = true;
		
		KHS_INFO(TEXT("레벨업! %d → %d (남은EXP: %.0f)"),	LevelToProcess, LevelToProcess + 1, CurrentExpToProcess);

		ApplyLevelUp(LevelToProcess, CurrentExpToProcess);
		SelectWeaponCandidates();
		
		bIsLevelingUp = false;

		LevelToProcess++;
	}
}

void ULevelUpSubsystem::SelectWeaponCandidates()
{
	UGameDataSubsystem* GDS = GetGameInstance()->GetSubsystem<UGameDataSubsystem>();
	check(GDS);

	TArray<FName> WeaponPool = GDS->GetWeaponIDsByLevel(1); //1레벨 무기만.

	if (WeaponPool.IsEmpty())
	{
		KHS_WARN(TEXT("무기 풀이 비어있음 — 레벨업 무기 선정 실패"));
		check(false);
		return;
	}

	Algo::RandomShuffle(WeaponPool);

	TArray<FName> Candidates;
	const int32 PickCount = FMath::Min(WeaponPool.Num(), WEAPON_CANDIDATE_COUNT);

	for (int32 i = 0; i < PickCount; i++)
	{
		Candidates.Add(WeaponPool[i]);
	}

	KHS_INFO(TEXT("무기 후보 선정 완료: %s / %s / %s"),
		*Candidates[0].ToString(),
		*Candidates[1].ToString(),
		*Candidates[2].ToString());

	// UI 미구현 구간 — 첫 번째 후보 자동 장착
	UEquipmentSubsystem* EquipSys = GetGameInstance()->GetSubsystem<UEquipmentSubsystem>();
	check(EquipSys);
	EquipSys->EquipWeapon(Candidates[0]);

	// 델리게이트 발행 — UI 연동 후 위 임시 코드 제거하고 여기서 처리
	OnWeaponCandidatesReadyDel.Broadcast(Candidates);
}

void ULevelUpSubsystem::ApplyLevelUp(int32 CurrentLevel, float OverflowEXP)
{
	ASC->SetNumericAttributeBase(AttributeSet->GetLevelAttribute(),	static_cast<float>(CurrentLevel + 1));
	ASC->SetNumericAttributeBase(AttributeSet->GetEXPAttribute(),OverflowEXP);

	KHS_INFO(TEXT("어트리뷰트 갱신 — Level: %d, EXP: %.0f"), CurrentLevel + 1, OverflowEXP);
}
