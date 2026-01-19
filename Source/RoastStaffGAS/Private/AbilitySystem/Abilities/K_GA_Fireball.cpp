// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/K_GA_Fireball.h"

#include <filesystem>

#include "AbilitySystem/GameplayTags/K_GameplayTags.h"
#include "Projectile/K_FireballProjectile.h"
#include "AbilitySystem/GameplayTags/K_GameplayTags.h"
#include "Character/K_BaseCharacter.h"

UK_GA_Fireball::UK_GA_Fireball()
{
	//GA 태그 설정
	FGameplayTagContainer tag;
	tag.AddTag(KTags::Ability_Skill_Fireball);
	SetAssetTags(tag);
	ActivationBlockedTags.AddTag(KTags::Ability_Skill_Fireball_Cooldown);
}

void UK_GA_Fireball::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	//CommitAbility 호출(Cost/Cooldown 적용)
	//내부에서 CommitCheck, CommitCost, CommitCooldown 체크
	//이 함수가 false를 반환하면 스킬 사용할수없는 상태.
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		LogAbilityInfo(TEXT("Fireball ActivateAbility failed"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	AK_FireballProjectile* projectile = SpawnFireBall(*ActorInfo);
	
	if (!projectile)
	{
		LogAbilityInfo(TEXT("Fireball SpawnFireBall failed"));
	}
	
	EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
}

void UK_GA_Fireball::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

AK_FireballProjectile* UK_GA_Fireball::SpawnFireBall(const FGameplayAbilityActorInfo& ActorInfo)
{
	check(ProjectileClass);
	
	AK_BaseCharacter* character = GetCharacterFromActorInfo(ActorInfo);
	if (!ensureMsgf(character,TEXT("[Fireball] Failed to get character from Actor Info")))
	{
		return nullptr;
	}
	
	UWorld* world = character->GetWorld();
	if (!ensureMsgf(world, TEXT("[Fireball] Failed to get World Info from charcter")))
	{
		return nullptr;
	}
	
	const FVector playerLoc = character->GetActorLocation();
	const FRotator playerRot = character->GetActorRotation();
	const FVector fwdVec = playerRot.Vector();
	
	//스폰위치 = 캐릭위치 + (전방*오프셋)
	const FVector spawnLoc = playerLoc + (fwdVec * SpawnOffset);
	
	FTransform spawnTransform;
	spawnTransform.SetLocation(spawnLoc);
	spawnTransform.SetRotation(playerRot.Quaternion());
	
	FActorSpawnParameters spawnParams;
	spawnParams.Owner = character;
	spawnParams.Instigator = character;
	spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	//Spawn Actor
	AK_FireballProjectile* projectile = world->SpawnActor<AK_FireballProjectile>(
		ProjectileClass, spawnTransform, spawnParams);
	
	//초기화
	if (projectile)
	{
		projectile->SetDamageInfo(BaseFireballDamage, character->GetAbilitySystemComponent());
	}
	
	return projectile;
}
