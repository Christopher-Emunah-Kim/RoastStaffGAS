// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/K_BaseGameplayAbility.h"
#include "RoastStaffGAS.h"
#include "Character/K_BaseCharacter.h"
#include "AbilitySystem/Attributes/K_BaseAttributeSet.h"
#include "System/K_LoggingSystem.h"

UK_BaseGameplayAbility::UK_BaseGameplayAbility()
{
	//기본 인스턴스 정책은 액터별 처리
	//-InstancedPerActor : 캐릭터당 하나 인스턴스
	//-InstancedPerExecution : 매 실행마다 인스턴스 생성
	//-NonInstanced : 인스턴스 없이 CDO사용
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	//멀티플레이 어빌리티 실행 정책은 클라이언트 예측 실행 후 서버 확정 방식
	//-LocalPredicted : 로컬 실행, 서버 확정
	//-LocalOnly : 클라에서만 실행(UI)
	//-ServerInitiated : 서버에서 시작해서 클라에 전파
	//-ServerOnly : 서버에서만.
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

AK_BaseCharacter* UK_BaseGameplayAbility::GetCharacterFromActorInfo(const FGameplayAbilityActorInfo& ActorInfo) const
{
	return Cast<AK_BaseCharacter>(ActorInfo.AvatarActor.Get());
}

UAbilitySystemComponent* UK_BaseGameplayAbility::GetOwnerASC() const
{
	return GetAbilitySystemComponentFromActorInfo_Ensured();
}

UK_BaseAttributeSet* UK_BaseGameplayAbility::GetOwnerBaseAttributeSet() const
{
	AK_BaseCharacter* character = Cast<AK_BaseCharacter>(GetAvatarActorFromActorInfo());
	if (!ensureMsgf(character, TEXT("이 GA를 소유한 character가 없음")))
	{
		return nullptr;
	}
	
	return character->GetAttributeSet();
}

void UK_BaseGameplayAbility::LogAbilityInfo(const FString& msg) const
{
	if (bLogActivated)
	{
		KHS_INFO(TEXT("[%s] %s"), *GetName(), *msg);
	}
}
