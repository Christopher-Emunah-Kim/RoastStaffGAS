// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/GA_CharacterSkill.h"
#include "RoastStaffGAS.h"
#include "GAS/Tags/RSGameplayTags.h"
#include "Objects/Data/RSCharacterSkillData.h"
#include "Subsystems/SkillManagerSubsystem.h"
#include "Subsystems/PoolingSubsystem.h"
#include "Character/BaseCharacter.h"
#include "Data/EnumTypes.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Objects/Projectile/BaseProjectile.h"
#include "Objects/GroundEffect/GroundEffectActor.h"
#include "System/LoggingSystem.h"


void UGA_CharacterSkill::OnAbilityActivated(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* ActorInfo,const FGameplayAbilityActivationInfo ActivationInfo)
{
	const URSCharacterSkillData* SkillData = Cast<URSCharacterSkillData>(GetCurrentSourceObject());
	if (!ensureMsgf(SkillData, TEXT("SourceObject가 URSCharacterSkillData가 아님")))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!ensureMsgf(SkillGEClass, TEXT("SkillGEClass 미설정 — BP에서 할당 필요")))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	GET_WORLD_SUBSYSTEM(USkillManagerSubsystem, SkillMgr)

	const FCharacterSkillExecData& ExecData = SkillMgr->GetSlotExecData(SkillData->SlotIndex);

	switch (ExecData.ActivationType)
	{
	case ESkillActivationType::InstantAoE:
		ExecuteInstantAoE(ExecData, Handle, ActorInfo, ActivationInfo);
		break;
	case ESkillActivationType::SelfBuff:
		ExecuteSelfBuff(ExecData, Handle, ActorInfo, ActivationInfo);
		break;
	case ESkillActivationType::SpawnPreview:
		ExecuteSpawnPreview(ExecData, Handle, ActorInfo, ActivationInfo);
		break;
	case ESkillActivationType::ProjectileSpawn:
		ExecuteProjectileSpawn(ExecData, Handle, ActorInfo, ActivationInfo);
		break;
	case ESkillActivationType::GroundEffect:
		ExecuteGroundEffect(ExecData, Handle, ActorInfo, ActivationInfo);
		break;
	default:
		KHS_WARN(TEXT("처리되지 않은 ActivationType — SlotIndex: %d"), SkillData->SlotIndex);
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		break;
	}
}

void UGA_CharacterSkill::ExecuteInstantAoE(	const FCharacterSkillExecData& ExecData, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,	const FGameplayAbilityActivationInfo ActivationInfo)
{
	if (!CachedInstigator)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const FVector Center = CachedInstigator->GetActorLocation();
	const float Radius   = FMath::Max(1.f, ExecData.LevelData.EffectRadius);

	// 범위 내 적 탐색
	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(CachedInstigator.Get());

	GetWorld()->OverlapMultiByChannel(Overlaps, Center, FQuat::Identity, ECC_Pawn,
		FCollisionShape::MakeSphere(Radius), QueryParams);

	UAbilitySystemComponent* OwnerASC = GetOwnerASC();

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* TargetActor = Overlap.GetActor();
		if (!TargetActor)
		{
			continue;
		}

		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
		if (!TargetASC || !TargetASC->HasMatchingGameplayTag(RSTags::Team_Enemy))
		{
			continue;
		}

		FGameplayEffectContextHandle Context = OwnerASC->MakeEffectContext();
		Context.AddInstigator(const_cast<ABaseCharacter*>(CachedInstigator.Get()), const_cast<ABaseCharacter*>(CachedInstigator.Get()));
		FGameplayEffectSpecHandle Spec = OwnerASC->MakeOutgoingSpec(SkillGEClass, 1, Context);
		if (!Spec.IsValid())
		{
			continue;
		}

		// ExecCalc 입력: BaseDamage = 10 × DamageMultiplier (기준 데미지 × 배율)
		Spec.Data->SetByCallerTagMagnitudes.Add(RSTags::Data_WeaponBaseDamage,10.f * ExecData.LevelData.DamageMultiplier);

		OwnerASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
	}

	SpawnSkillFX(ExecData.LevelData.FXClass, Center, Radius, ExecData.ElementTag);

	KHS_INFO(TEXT("InstantAoE 발동 — SkillID: %s | 반경: %.0f"), *ExecData.SkillID.ToString(), Radius);
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UGA_CharacterSkill::ExecuteSelfBuff(const FCharacterSkillExecData& ExecData, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,	const FGameplayAbilityActivationInfo ActivationInfo)
{
	UAbilitySystemComponent* OwnerASC = GetOwnerASC();

	FGameplayEffectContextHandle Context = OwnerASC->MakeEffectContext();
	Context.AddInstigator(const_cast<ABaseCharacter*>(CachedInstigator.Get()), const_cast<ABaseCharacter*>(CachedInstigator.Get()));
	FGameplayEffectSpecHandle Spec = OwnerASC->MakeOutgoingSpec(SkillGEClass, 1, Context);

	if (!Spec.IsValid())
	{
		KHS_WARN(TEXT("GE 스펙 생성 실패 — SkillID: %s"), *ExecData.SkillID.ToString());
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Duration은 GE BP에서 설정 — 여기서는 SetByCaller로 Duration 오버라이드 불필요
	OwnerASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());

	if (CachedInstigator)
	{
		SpawnSkillFX(ExecData.LevelData.FXClass, CachedInstigator->GetActorLocation(), ExecData.LevelData.EffectRadius);
	}

	KHS_INFO(TEXT("SelfBuff 발동 — SkillID: %s | Duration: %.1fs"), *ExecData.SkillID.ToString(), ExecData.LevelData.Duration);
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UGA_CharacterSkill::ExecuteSpawnPreview(const FCharacterSkillExecData& ExecData, const FGameplayAbilitySpecHandle Handle,	const FGameplayAbilityActorInfo* ActorInfo,	const FGameplayAbilityActivationInfo ActivationInfo)
{
	GET_WORLD_SUBSYSTEM(USkillManagerSubsystem, SkillMgr)
	const FVector TargetLoc = SkillMgr->GetPendingTargetLocation();
	const float Radius      = FMath::Max(1.f, ExecData.LevelData.EffectRadius);

	// InstantAoE와 동일하게 처리 — 단, 중심이 PendingTargetLocation
	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams QueryParams;
	if (CachedInstigator)
	{
		QueryParams.AddIgnoredActor(CachedInstigator.Get());
	}

	GetWorld()->OverlapMultiByChannel(Overlaps, TargetLoc, FQuat::Identity, ECC_Pawn,
		FCollisionShape::MakeSphere(Radius), QueryParams);

	UAbilitySystemComponent* OwnerASC = GetOwnerASC();

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* TargetActor = Overlap.GetActor();
		if (!TargetActor)
		{
			continue;
		}

		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
		if (!TargetASC || !TargetASC->HasMatchingGameplayTag(RSTags::Team_Enemy))
		{
			continue;
		}

		FGameplayEffectContextHandle Context = OwnerASC->MakeEffectContext();
		Context.AddInstigator(const_cast<ABaseCharacter*>(CachedInstigator.Get()), const_cast<ABaseCharacter*>(CachedInstigator.Get()));
		FGameplayEffectSpecHandle Spec = OwnerASC->MakeOutgoingSpec(SkillGEClass, 1, Context);
		if (!Spec.IsValid())
		{
			continue;
		}

		Spec.Data->SetByCallerTagMagnitudes.Add(RSTags::Data_WeaponBaseDamage,
			10.f * ExecData.LevelData.DamageMultiplier);

		OwnerASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
	}

	SpawnSkillFX(ExecData.LevelData.FXClass, TargetLoc, Radius, ExecData.ElementTag);

	KHS_INFO(TEXT("SpawnPreview AoE 발동 — SkillID: %s | 위치: %s"), *ExecData.SkillID.ToString(), *TargetLoc.ToString());
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UGA_CharacterSkill::ExecuteProjectileSpawn(
	const FCharacterSkillExecData& ExecData,
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	TSubclassOf<ABaseProjectile> LoadedClass;
	if (!LoadRequiredClass(ExecData.ProjectileClass, LoadedClass, ExecData.SkillID))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (ExecData.ProjectileCount <= 1)
	{
		// 단발 — 즉시 발사 후 종료
		FireOneProjectile(LoadedClass, ExecData);
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	// 연속 발사 (burst) — 첫 발 즉시, 이후 타이머
	ActiveProjClass      = LoadedClass;
	CachedProjExecData   = ExecData;
	RemainingFireCount   = ExecData.ProjectileCount - 1;

	FireOneProjectile(LoadedClass, ExecData);

	TWeakObjectPtr<UGA_CharacterSkill> WeakThis(this);
	GetWorld()->GetTimerManager().SetTimer(
		MultiFireTimerHandle,
		[WeakThis]()
		{
			if (!WeakThis.IsValid())
			{
				return;
			}
			WeakThis->FireOneProjectile(WeakThis->ActiveProjClass, WeakThis->CachedProjExecData);
			WeakThis->RemainingFireCount--;

			if (WeakThis->RemainingFireCount <= 0)
			{
				WeakThis->GetWorld()->GetTimerManager().ClearTimer(WeakThis->MultiFireTimerHandle);
				WeakThis->EndAbility(
					WeakThis->GetCurrentAbilitySpecHandle(),
					WeakThis->GetCurrentActorInfo(),
					WeakThis->GetCurrentActivationInfo(),
					true, false
				);
			}
		},
		ExecData.FireInterval,
		true
	);

	KHS_INFO(TEXT("ProjectileSpawn 연속 발사 시작 — SkillID: %s | %d발 × %.2fs"), *ExecData.SkillID.ToString(), ExecData.ProjectileCount, ExecData.FireInterval);
}

void UGA_CharacterSkill::FireOneProjectile(TSubclassOf<ABaseProjectile> ProjClass, const FCharacterSkillExecData& ExecData)
{
	FProjectileInitData InitData;
	InitData.SkillID       = ExecData.SkillID;
	InitData.SkillEffectID = ExecData.SkillEffectID;
	InitData.DamageGEClass = SkillGEClass;
	InitData.InstigatorASC = GetOwnerASC();
	InitData.Amount        = ExecData.Amount * ExecData.LevelData.DamageMultiplier;
	InitData.Speed         = ExecData.ProjectileSpeed;
	InitData.Lifetime      = ExecData.ProjectileLifetime;
	InitData.SpawnPattern  = ExecData.SpawnPattern;
	InitData.MoveType      = ExecData.MoveType;
	InitData.HitType       = ExecData.HitType;
	InitData.SpawnCount    = 1;
	InitData.PierceCount   = ExecData.PierceCount;
	InitData.DamageDecay   = ExecData.DamageDecay;
	InitData.SpreadAngle   = 0.f;

	SpawnProjectiles(ProjClass, InitData);

	if (CachedInstigator)
	{
		SpawnSkillFX(ExecData.LevelData.FXClass, CachedInstigator->GetActorLocation(), ExecData.LevelData.EffectRadius);
	}

	KHS_INFO(TEXT("ProjectileSpawn 단발 — SkillID: %s"), *ExecData.SkillID.ToString());
}

void UGA_CharacterSkill::SpawnSkillFX(TSoftObjectPtr<UNiagaraSystem> FXClass, FVector Location, float Radius, FGameplayTag ElementTag)
{
	UNiagaraSystem* FX = FXClass.LoadSynchronous();
	if (!FX)
	{
		return;
	}

	UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(), FX, Location, FRotator::ZeroRotator, FVector(1.f), true, true);

	if (!NiagaraComp)
	{
		return;
	}

	NiagaraComp->SetVariableFloat(FName(TEXT("Radius")), Radius);

	// ElementTag → ElementColor 분기 (Niagara FX에 "ElementColor" LinearColor 파라미터 필수)
	FLinearColor ElementColor = FLinearColor::White;
	if (ElementTag == RSTags::Element_Fire)
	{
		ElementColor = FLinearColor(1.f, 0.3f, 0.f, 1.f);
	}
	else if (ElementTag == RSTags::Element_Ice)
	{
		ElementColor = FLinearColor(0.3f, 0.8f, 1.f, 1.f);
	}
	else if (ElementTag == RSTags::Element_Thunder)
	{
		ElementColor = FLinearColor(1.f, 1.f, 0.f, 1.f);
	}
	NiagaraComp->SetVariableLinearColor(FName(TEXT("ElementColor")), ElementColor);

	// 루프 이펙트 자동 제거 — DESTROY_FX_DELAY 후 Deactivate (bAutoDestroy=true이므로 즉시 파괴)
	TWeakObjectPtr<UNiagaraComponent> WeakComp(NiagaraComp);
	FTimerHandle FXLifetimeHandle;
	GetWorld()->GetTimerManager().SetTimer(FXLifetimeHandle, [WeakComp]()
	{
		if (WeakComp.IsValid())
		{
			WeakComp->Deactivate();
		}
	}, DESTROY_FX_DELAY, false);
}

void UGA_CharacterSkill::ExecuteGroundEffect(
	const FCharacterSkillExecData& ExecData,
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	TSubclassOf<AGroundEffectActor> GroundClass;
	if (!LoadRequiredClass(ExecData.GroundEffectActorClass, GroundClass, ExecData.SkillID))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	GET_WORLD_SUBSYSTEM(UPoolingSubsystem, PoolSub)
	GET_WORLD_SUBSYSTEM(USkillManagerSubsystem, SkillMgr)
	const FVector SpawnLoc = SkillMgr->GetPendingTargetLocation();

	AGroundEffectActor* GroundActor = PoolSub->SpawnPooledActor<AGroundEffectActor>(
		GroundClass, FTransform(FRotator::ZeroRotator, SpawnLoc));

	if (!GroundActor)
	{
		KHS_WARN(TEXT("GroundEffect 스폰 실패 — SkillID: %s"), *ExecData.SkillID.ToString());
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const float FinalAmount = ExecData.Amount * ExecData.LevelData.DamageMultiplier;
	GroundActor->InitGroundEffect(
		GetOwnerASC(),
		ExecData.LevelData.Duration,
		SkillGEClass,
		ExecData.LevelData.EffectRadius,
		ExecData.LevelData.FXClass,
		FinalAmount);

	KHS_INFO(TEXT("GroundEffect 발동 — SkillID: %s | 위치: %s | Duration: %.1fs"),
		*ExecData.SkillID.ToString(), *SpawnLoc.ToString(), ExecData.LevelData.Duration);

	// 장판 Actor가 독립 수명 관리 — GA는 즉시 종료
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
