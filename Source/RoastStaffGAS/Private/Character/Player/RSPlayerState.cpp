// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/RSPlayerState.h"
#include "AbilitySystemComponent.h"
#include "RoastStaffGAS.h"
#include "GAS/Attributes/PlayerAttributeSet.h"
#include "GAS/Tags/RSGameplayTags.h"
#include "System/LoggingSystem.h"
#include "Subsystems/GameDataSubsystem.h"
#include "Data/DataTableStructs.h"

ARSPlayerState::ARSPlayerState()
{
	// ASC 생성 — PlayerState가 Owner
	ASC = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("ASC"));

	// PlayerAttributeSet 생성 및 ASC 등록
	PlayerAttributeSet = CreateDefaultSubobject<UPlayerAttributeSet>(TEXT("PlayerAttributeSet"));
	ASC->AddAttributeSetSubobject<UPlayerAttributeSet>(PlayerAttributeSet);
}

void ARSPlayerState::InitializeAbilitySystem(AActor* AvatarActor)
{
	if (bIsInitialized)
	{
		KHS_WARN(TEXT("이미 초기화됨. 중복 호출 무시."));
		return;
	}

	if (!ensureMsgf(ASC, TEXT("ASC가 null.")))
	{
		return;
	}

	if (!ensureMsgf(AvatarActor, TEXT("AvatarActor가 null.")))
	{
		return;
	}

	// Owner = PlayerState (권한/RPC 기준)
	// Avatar = PlayerCharacter (물리적 실행 담당)
	ASC->InitAbilityActorInfo(this, AvatarActor);

	// 플레이어 팀 태그 부여
	FGameplayTagContainer PlayerTag;
	PlayerTag.AddTag(RSTags::Team_Player);
	ASC->AddLooseGameplayTags(PlayerTag);

	// 실제 스탯은 RSGameMode::BeginPlay에서 ApplyCharacterStats(CharID)로 주입됨
	// 어트리뷰트는 PlayerAttributeSet 생성자 기본값(HP=0, Level=1 등)으로 유지

	bIsInitialized = true;

	KHS_INFO(TEXT("초기화 완료. Avatar: %s"), *AvatarActor->GetName());
}

void ARSPlayerState::ApplyCharacterStats(FName CharID)
{
	if (!ensureMsgf(ASC, TEXT("ASC가 null.")))
	{
		return;
	}
	if (!ensureMsgf(PlayerAttributeSet, TEXT("PlayerAttributeSet이 null.")))
	{
		return;
	}
	if (CharID.IsNone())
	{
		KHS_WARN(TEXT(" CharID가 NAME_None. 스탯 적용 건너뜀."));
		return;
	}

	GET_GI_SUBSYSTEM(UGameDataSubsystem, GDS);

	FCharacterStaticData Data;
	if (!GDS->GetCharacterStaticData(CharID, Data))
	{
		KHS_WARN(TEXT("CharID [%s] 데이터 조회 실패. 스탯 적용 건너뜀."), *CharID.ToString());
		return;
	}

	ASC->SetNumericAttributeBase(PlayerAttributeSet->GetMaxHPAttribute(),          Data.BaseHP);
	ASC->SetNumericAttributeBase(PlayerAttributeSet->GetCurrentHPAttribute(),      Data.BaseHP);
	ASC->SetNumericAttributeBase(PlayerAttributeSet->GetMoveSpeedAttribute(),      Data.BaseMoveSpeed);
	ASC->SetNumericAttributeBase(PlayerAttributeSet->GetATKAttribute(),            Data.BaseATK);
	ASC->SetNumericAttributeBase(PlayerAttributeSet->GetDEFAttribute(),            Data.BaseDEF);
	ASC->SetNumericAttributeBase(PlayerAttributeSet->GetAttackSpeedAttribute(),    Data.BaseAttackSpeed);
	ASC->SetNumericAttributeBase(PlayerAttributeSet->GetCastingSpeedAttribute(),   Data.BaseCastingSpeed);
	ASC->SetNumericAttributeBase(PlayerAttributeSet->GetCriticalRateAttribute(),   Data.BaseCriticalRate);
	ASC->SetNumericAttributeBase(PlayerAttributeSet->GetCriticalDamageAttribute(), Data.BaseCriticalDamage);
	ASC->SetNumericAttributeBase(PlayerAttributeSet->GetLevelAttribute(),          1.f);

	KHS_INFO(TEXT("캐릭터 스탯 적용 완료 — CharID: %s | HP: %.0f / SPD: %.0f / ATK: %.0f / DEF: %.0f"),
		*CharID.ToString(), Data.BaseHP, Data.BaseMoveSpeed, Data.BaseATK, Data.BaseDEF);
}
