// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/GA_Base.h"

#include "RoastStaffGAS.h"
#include "Character/BaseCharacter.h"
#include "Data/EnumTypes.h"
#include "Objects/Data/RSSkillData.h"
#include "Objects/Projectile/BaseProjectile.h"
#include "Subsystems/PoolingSubsystem.h"


UGA_Base::UGA_Base()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGA_Base::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	// Instigator 캐싱 — 투사체 스폰 방향 계산에 사용
	if (TriggerEventData)
	{
		CachedInstigator = Cast<ABaseCharacter>(TriggerEventData->Instigator.Get());
	}

	if (!CachedInstigator)
	{
		CachedInstigator = Cast<ABaseCharacter>(ActorInfo->AvatarActor.Get());
	}

	OnAbilityActivated(Handle, ActorInfo, ActivationInfo);
}

void UGA_Base::OnAbilityActivated(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	//자식 GA에서 오버라이딩하여 사용
}

void UGA_Base::SpawnProjectiles(TSubclassOf<ABaseProjectile> ProjectileClass, const FProjectileInitData& InitData)
{
	if (!ensureMsgf(CachedInstigator, TEXT("CachedInstigator is null")))
	{
		return;
	}
	
	if (!ensureMsgf(ProjectileClass, TEXT("ProjectileClass is null")))
	{
		return;
	}
	
	const int32 Count = FMath::Max(1, InitData.SpawnCount);
	
	// ProjectileCount=1인데 FanSpread면 경고 후 Single로 처리
	if (Count == 1 && InitData.SpawnPattern == ESpawnPattern::SPREAD)
	{
		KHS_WARN(TEXT("Count=1이지만 Spread 패턴 — Single로 처리"));
	}
	
	GET_WORLD_SUBSYSTEM(UPoolingSubsystem, PoolSys);
	
	for (int32 i = 0; i < Count; ++i)
	{
		// 스폰 위치 — 캐릭터 전방 N미터
		FVector SpawnLocation = CachedInstigator->GetActorLocation() + CachedInstigator->GetActorForwardVector() * SPAWN_OFFSET;
		FRotator SpawnRotation = CachedInstigator->GetActorRotation();
	
		// SpawnPattern별 방향 계산
		switch (InitData.SpawnPattern)
		{
		case ESpawnPattern::SPREAD:
			{
				if (Count > 1)
				{
					float HalfAngle = InitData.SpreadAngle * 0.5f;
					float Step = InitData.SpreadAngle / (Count - 1);
					SpawnRotation.Yaw += -HalfAngle + Step * i;
				}
				break;
			}
		case ESpawnPattern::CIRCLE:
			{
				SpawnRotation.Yaw += (360.f / Count) * i;
				break;
			}
		case ESpawnPattern::SINGLE:
		default:
			break;
		}
	
		ABaseProjectile* Projectile = PoolSys->SpawnPooledActor<ABaseProjectile>(ProjectileClass, FTransform(SpawnRotation,SpawnLocation));
	
		if (!Projectile)
		{
			KHS_WARN(TEXT("SpawnPooledActor 실패 — 인덱스 %d"), i);
			continue;
		}
		
		Projectile->SetOwner(const_cast<ABaseCharacter*>(CachedInstigator.Get()));
		Projectile->SetInstigator(const_cast<ABaseCharacter*>(CachedInstigator.Get()));
		Projectile->InitProjectile(InitData);
	}
}

UAbilitySystemComponent* UGA_Base::GetOwnerASC() const
{
	return GetAbilitySystemComponentFromActorInfo_Ensured();
}
