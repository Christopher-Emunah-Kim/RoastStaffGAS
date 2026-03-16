// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/RSPlayerState.h"
#include "AbilitySystemComponent.h"
#include "GAS/Attributes/PlayerAttributeSet.h"
#include "GAS/Tags/RSGameplayTags.h"
#include "System/LoggingSystem.h"

ARSPlayerState::ARSPlayerState()
{
	// ASC 생성 — PlayerState가 Owner
	ASC = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("ASC"));
	//ASC->SetIsReplicated(true);
	//ASC->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	// PlayerAttributeSet 생성 및 ASC 등록
	PlayerAttributeSet = CreateDefaultSubobject<UPlayerAttributeSet>(TEXT("PlayerAttributeSet"));
	ASC->AddAttributeSetSubobject<UPlayerAttributeSet>(PlayerAttributeSet);
	//SetNetUpdateFrequency(100.f);
}

void ARSPlayerState::ApplyBaseStats()
{
	if (!ensureMsgf(PlayerAttributeSet, TEXT("ApplyBaseStats 실패 — PlayerAttributeSet이 null.")))
	{
		return;
	}

	// TODO: GDS에서 DT_Character 조회 후 실제 스탯으로 교체
	// UGameDataSubsystem* GDS = GetGameInstance()->GetSubsystem<UGameDataSubsystem>();
	// FCharacterStaticData Data;
	// GDS->GetCharacterData(CharacterID, Data);

	// 더미값 — DT_Character 미구현으로 임시 하드코딩
	ASC->SetNumericAttributeBase(PlayerAttributeSet->GetMaxHPAttribute(),      200.f);
	ASC->SetNumericAttributeBase(PlayerAttributeSet->GetCurrentHPAttribute(),  200.f);
	ASC->SetNumericAttributeBase(PlayerAttributeSet->GetMoveSpeedAttribute(),  400.f);
	ASC->SetNumericAttributeBase(PlayerAttributeSet->GetATKAttribute(),         20.f);
	ASC->SetNumericAttributeBase(PlayerAttributeSet->GetDEFAttribute(),          5.f);
	ASC->SetNumericAttributeBase(PlayerAttributeSet->GetAttackSpeedAttribute(),  1.f);
	ASC->SetNumericAttributeBase(PlayerAttributeSet->GetCastingSpeedAttribute(), 1.f);
	ASC->SetNumericAttributeBase(PlayerAttributeSet->GetCriticalRateAttribute(), 0.05f);
	ASC->SetNumericAttributeBase(PlayerAttributeSet->GetCriticalDamageAttribute(), 1.5f);
	ASC->SetNumericAttributeBase(PlayerAttributeSet->GetLevelAttribute(),        1.f);

	KHS_INFO(TEXT("플레이어 기본 스탯 주입 완료"));
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

	// 스탯 기본값 주입
	ApplyBaseStats();
	
	// 플레이어 팀 태그 부여 
	FGameplayTagContainer PlayerTag;
	PlayerTag.AddTag(RSTags::Team_Player);
	ASC->AddLooseGameplayTags(PlayerTag);

	bIsInitialized = true;

	KHS_INFO(TEXT("초기화 완료. Avatar: %s"), *AvatarActor->GetName());
}
