// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/K_GA_BasicShoot.h"

#include "TwinStickProjectile.h"
#include "AbilitySystem/GameplayTags/K_GameplayTags.h"
#include "Character/K_BaseCharacter.h"
#include "System/K_LoggingSystem.h"

UK_GA_BasicShoot::UK_GA_BasicShoot()
{
	//AbilityTag/BlockedTag 설정
	AbilityTags.AddTag(KTags::Ability_Combat_BasicShoot);
	ActivationBlockedTags.AddTag(KTags::Ability_Combat_BasicShoot);
}

void UK_GA_BasicShoot::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	//CommitAbility
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		KHS_WARN(TEXT("[BasicShoot] - CommitAbility is Failed"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	//발사체 스폰
	//TODO 나중에 발사체 구현후 수정
	ATwinStickProjectile* projectile = SpawnBasicProjectile(*ActorInfo);
	if (!ensureMsgf(projectile, TEXT("projectile spawn is failed")))
	{
		//발사체 생성 실패해도 일단 정상 종료까지 진행(Commit에서 Cost차감했음)
	}
	
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UK_GA_BasicShoot::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	
	
}

ATwinStickProjectile* UK_GA_BasicShoot::SpawnBasicProjectile(const FGameplayAbilityActorInfo& ActorInfo)
{
	//TODO 이후 발사체 클래스 만들고 변경
	if (!ensureMsgf(ProjectileClass, TEXT("[BasicShoot] ProjectileClass is not set in Blueprint!")))
	{
		return nullptr;
	}
	
	AK_BaseCharacter* character = GetCharacterFromActorInfo(ActorInfo);
	if (!ensureMsgf(character, TEXT("[BasicShoot] failed to get character from actor info")))
	{
		return nullptr;
	}
	
	UWorld* world = character->GetWorld();
	if (!ensureMsgf(world, TEXT("[BasicShoot] failed to get world from character class")))
	{
		return nullptr;
	}
	
	//Spawn Setting
	const FVector characterLocation = character->GetActorLocation();
	const FRotator characterRotation = character->GetActorRotation();
	const FVector forwardVector = characterRotation.Vector();
	const FVector spawnLocation = characterLocation + (forwardVector * SpawnOffset);

	FTransform spawnTransform;
	spawnTransform.SetLocation(spawnLocation);
	spawnTransform.SetRotation(characterRotation.Quaternion());
	
	FActorSpawnParameters spawnInfo;
	spawnInfo.Owner = character;
	spawnInfo.Instigator = character;
	spawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	//Spawn
	ATwinStickProjectile* projectile = world->SpawnActor<ATwinStickProjectile>(ProjectileClass, spawnTransform, spawnInfo);
	if (!ensureMsgf(projectile, TEXT("[BasicShoot] failed to spawn projectile")))
	{
		return nullptr;
	}
	
	UAbilitySystemComponent* ownerASC = character->GetAbilitySystemComponent();
	//TODO 발사체 구현후 주석 풀기
	//projectile->SetDamageInfo(BaseDamage, ownerASC);
	
	KHS_INFO(TEXT("[BasicShoot] Projectile spawned - Damage: %.1f"), BaseDamage);
	
	return projectile;
}
