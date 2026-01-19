// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/K_PlayerState.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/K_BaseAttributeSet.h"
#include "System//K_LoggingSystem.h"

AK_PlayerState::AK_PlayerState()
{
	//ASC생성 
	//PlyaerState가 Owner
	//ReplicationMode(Mixed) - 플레이어는 Full, 다른 클라이언트엔 Minimal
	ASC = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("ASC"));
	ASC->SetIsReplicated(true);
	ASC->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	
	//AttributeSet 생성 및 등록
	BaseAttributeSet = CreateDefaultSubobject<UK_BaseAttributeSet>(TEXT("BaseAttributeSet"));
	ASC->AddAttributeSetSubobject<UK_BaseAttributeSet>(BaseAttributeSet);
	
	//PlayerState의 넷업데이트 빈도를 ASC사용해야하니까 높여주기
	SetNetUpdateFrequency(100.f);
}

UAbilitySystemComponent* AK_PlayerState::GetAbilitySystemComponent() const
{
	return ASC;
}

void AK_PlayerState::InitializeAbilities(AActor* AvatarActor)
{
	//중복 초기화 방지
	if (bAbilitiesInitialized)
	{
		KHS_WARN(TEXT("[PlayerState] Abilities already initialized"));
		return;
	}

	if (!ASC || !AvatarActor)
	{
		KHS_WARN(TEXT("[PlayerState] ASC or AvatarActor is not valid"));
		return;
	}
	
	//Owner - PlayerState (RPC와 권한 기준) / Avatar : Player(물리적 실행 담당)
	ASC->InitAbilityActorInfo(this, AvatarActor);
	
	//서버에서만 어빌리티 부여
	//클라는 리플리케이션으로 받음
	if (GetLocalRole() == ROLE_Authority)
	{
		for (const TSubclassOf<UGameplayAbility>& ability : InitialAbilities)
		{
			if (ability)
			{
				FGameplayAbilitySpec spec(ability, CharacterLevel, INDEX_NONE, this);
				ASC->GiveAbility(spec);
			}
		}
		
		KHS_INFO(TEXT("[PlayerState] Abilities granted on server: %d"), InitialAbilities.Num());
	}
	
	bAbilitiesInitialized = true;
	KHS_INFO(TEXT("[PlayerState] AbilitySystem initialized. Owner: %s, Avatar: %s"), 
		*GetName(), *AvatarActor->GetName());
}

