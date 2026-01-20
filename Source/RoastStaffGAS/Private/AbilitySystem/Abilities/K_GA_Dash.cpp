// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/K_GA_Dash.h"

#include "AbilitySystem/GameplayTags/K_GameplayTags.h"
#include "Character/K_PlayerCharacter.h"
#include "EntitySystem/MovieSceneEntitySystemRunner.h"
#include "System/K_LoggingSystem.h"


UK_GA_Dash::UK_GA_Dash()
{
	FGameplayTagContainer tag;
	tag.AddTag(KTags::Ability_Movement_Dash);
	SetAssetTags(tag);
	ActivationBlockedTags.AddTag(KTags::Ability_Movement_Dash_Cooldown);
	ActivationBlockedTags.AddTag(KTags::State_Dashing);
}

void UK_GA_Dash::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		LogAbilityInfo(TEXT("Dash CommitAbility Failed"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	//GE적용
	if (!ensureMsgf(DashEffect, TEXT("[GA_Dash] Gameeffect is not set on BP")))
	{
		return;
	}
	FGameplayEffectSpecHandle specHandle = MakeOutgoingGameplayEffectSpec(DashEffect, 1.f);
	if (!specHandle.IsValid())
	{
		KHS_WARN(TEXT("[GA_Dash] Invalid GE spec Handle"));
	}
		
	//AcitvateHandle값을 반환하지만 지금은 쓸곳없으니 무시
	(void)ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, specHandle);
	
	//캐릭 가져오기
	AK_BaseCharacter* character = GetCharacterFromActorInfo(*ActorInfo);
	if (!ensureMsgf(character, TEXT("Failied to Get Character")))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	//대쉬 동작
	AK_PlayerCharacter* player = CastChecked<AK_PlayerCharacter>(character);
	FVector dashDirection = player->GetDashDirection();
	FVector launchVelocity = dashDirection * DashImpulse;
	
	character->LaunchCharacter(launchVelocity, true, true);
	
	//LogAbilityInfo(TEXT("Dash activated"));
	
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UK_GA_Dash::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
