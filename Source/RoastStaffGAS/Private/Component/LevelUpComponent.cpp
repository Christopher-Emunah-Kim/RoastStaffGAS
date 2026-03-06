// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/LevelUpComponent.h"
#include "AbilitySystemComponent.h"
#include "GAS/Attributes/PlayerAttributeSet.h"
#include "Component/EquipmentComponent.h"
#include "Subsystems/GameDataSubsystem.h"
#include "Data/DataTableStructs.h"
#include "GAS/Tags/RSGameplayTags.h"
#include "System/LoggingSystem.h"
#include "Engine/GameInstance.h"
#include "GameplayEffect.h"
#include "Algo/RandomShuffle.h"


ULevelUpComponent::ULevelUpComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void ULevelUpComponent::Initialize(UAbilitySystemComponent* InASC, UPlayerAttributeSet* InAttributeSet,
	UEquipmentComponent* InEquipmentComp)
{
	if (!InASC || !InAttributeSet || !InEquipmentComp)
	{
		KHS_WARN(TEXT("Invalid Parameters"));
		return;
	}
	
	ASC            = InASC;
	AttributeSet   = InAttributeSet;
	EquipmentComp  = InEquipmentComp;

	// EXP 변경 델리게이트 구독
	AttributeSet->OnEXPChangedDel.AddDynamic(this, &ULevelUpComponent::OnEXPChanged);

	KHS_INFO(TEXT("LevelUpComponent 초기화 완료"));
}

void ULevelUpComponent::AddEXP(float Amount)
{
	if (!ensureMsgf(ASC, TEXT("ASC is null")))
	{
		return;
	}

	if (Amount <= 0.f)
	{
		KHS_WARN(TEXT("AddEXP: Amount가 0 이하 — 무시"));
		return;
	}

	const int32 CurrentLevel = static_cast<int32>(AttributeSet->GetLevel());
	
	if (CurrentLevel >= MAX_LEVEL)
	{
		KHS_INFO(TEXT("최대 레벨 도달 — EXP 누적 중단"));
		return;
	}

	// GE_AddEXP로 EXP 가산
	UGameInstance* GI = GetWorld()->GetGameInstance();
	check(GI);
	UGameDataSubsystem* GDS = GI->GetSubsystem<UGameDataSubsystem>();
	check(GDS);

	// TODO: GE_AddEXP 클래스를 DataTable로 관리
	// 지금은 컴포넌트 UPROPERTY로 에디터에서 직접 지정
	if (!ensureMsgf(AddEXPEffectClass, TEXT("AddEXPEffectClass가 설정되지 않음")))
	{
		return;
	}

	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(AddEXPEffectClass, 1.f, Context);

	if (!Spec.IsValid())
	{
		KHS_WARN(TEXT("GE_AddEXP Spec 생성 실패"));
		return;
	}

	Spec.Data->SetByCallerTagMagnitudes.Add(RSTags::Data_EXP, Amount);
	ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());

	KHS_INFO(TEXT("EXP 추가: +%.0f"), Amount);
}

void ULevelUpComponent::OnEXPChanged(float NewEXP, int32 CurrentLevel)
{
	CheckLevelUp(NewEXP, CurrentLevel);
}

void ULevelUpComponent::CheckLevelUp(float NewEXP, int32 CurrentLevel)
{
	if (bIsLevelingUp || CurrentLevel >= MAX_LEVEL)
	{
		return;
	}
	
	UGameInstance* GI = GetWorld()->GetGameInstance();
	check(GI);
	UGameDataSubsystem* GDS = GI->GetSubsystem<UGameDataSubsystem>();
	check(GDS);

	float CurrentExpToProcess = NewEXP;
	int32 LevelToProcess = CurrentLevel;
	float RequiredExp = 0.f;
	
	// while문을 통해 경험치가 요구량보다 많은 동안 계속 레벨업 처리 
	while (LevelToProcess < MAX_LEVEL && GDS->GetLevelCurveValue(FName("RequiredEXP"), LevelToProcess + 1, RequiredExp) && CurrentExpToProcess >= RequiredExp)
	{
		CurrentExpToProcess -= RequiredExp; // 잉여 EXP 갱신
		
		bIsLevelingUp = true;
		KHS_INFO(TEXT("레벨업! %d → %d (남은EXP: %.0f)"), LevelToProcess, LevelToProcess + 1, CurrentExpToProcess);

		ApplyLevelUp(LevelToProcess, CurrentExpToProcess);
		SelectWeaponCandidates();
		bIsLevelingUp = false;

		LevelToProcess++; // 다음 레벨 검사를 위해 레벨 증가
	}
}

void ULevelUpComponent::SelectWeaponCandidates()
{
	UGameInstance* GI = GetWorld()->GetGameInstance();
	check(GI);
	UGameDataSubsystem* GDS = GI->GetSubsystem<UGameDataSubsystem>();
	check(GDS);

	// WeaponLevel=1 풀에서 랜덤 선정(1레벨 무기만)
	TArray<FName> WeaponPool = GDS->GetWeaponIDsByLevel(1);

	if (WeaponPool.IsEmpty())
	{
		KHS_WARN(TEXT("무기 풀이 비어있음 — 레벨업 무기 선정 실패"));
		return;
	}

	//원본 배열 무작위 섞기
	Algo::RandomShuffle(WeaponPool);
	TArray<FName> Candidates;
	const int32 CANDIDATE_COUNT = 3;
	
	//풀 크기랑 요구 갯수중 더 작은 값만 앞에서부터 추출.
	const int32 PickCount = FMath::Min(WeaponPool.Num(), CANDIDATE_COUNT);
	for (int32 i = 0; i < PickCount; i++)
	{
		Candidates.Add(WeaponPool[i]);
	}

	KHS_INFO(TEXT("무기 후보 선정 완료: %s / %s / %s"),
		*Candidates[0].ToString(), *Candidates[1].ToString(), *Candidates[2].ToString());

	// 델리게이트 발행 — 추후 UI가 구독
	OnWeaponCandidatesReadyDel.Broadcast(Candidates);

	// 첫 번째 후보 자동 장착 — UI 연동 후 제거
	EquipmentComp->EquipWeapon(Candidates[0]);
}

void ULevelUpComponent::ApplyLevelUp(int32 CurrentLevel, float OverflowEXP)
{
	// Level +1
	ASC->SetNumericAttributeBase(AttributeSet->GetLevelAttribute(),	static_cast<float>(CurrentLevel + 1));

	// EXP 잉여분으로 재세팅
	ASC->SetNumericAttributeBase(AttributeSet->GetEXPAttribute(), OverflowEXP);

	KHS_INFO(TEXT("어트리뷰트 갱신 — Level: %d, EXP: %.0f"), CurrentLevel + 1, OverflowEXP);
}

