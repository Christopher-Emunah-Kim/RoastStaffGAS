// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/GA_CharacterSkill.h"
#include "RoastStaffGAS.h"
#include "GAS/Tags/RSGameplayTags.h"
#include "Objects/Data/RSCharacterSkillData.h"
#include "Subsystems/SkillManagerSubsystem.h"
#include "Character/BaseCharacter.h"
#include "Data/EnumTypes.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
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

	USkillManagerSubsystem* SkillMgr = GetWorld()->GetSubsystem<USkillManagerSubsystem>();
	if (!ensureMsgf(SkillMgr, TEXT(" SkillManagerSubsystem 조회 실패")))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

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

	if (UNiagaraSystem* FX = ExecData.LevelData.FXClass.LoadSynchronous())
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), FX, Center);
	}

	SpawnSkillFX(ExecData.LevelData.FXClass, Center, Radius);

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
	USkillManagerSubsystem* SkillMgr = GetWorld()->GetSubsystem<USkillManagerSubsystem>();
	const FVector TargetLoc = SkillMgr ? SkillMgr->GetPendingTargetLocation() : FVector::ZeroVector;
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

	SpawnSkillFX(ExecData.LevelData.FXClass, TargetLoc, Radius);

	KHS_INFO(TEXT("SpawnPreview AoE 발동 — SkillID: %s | 위치: %s"), *ExecData.SkillID.ToString(), *TargetLoc.ToString());
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UGA_CharacterSkill::SpawnSkillFX(TSoftObjectPtr<UNiagaraSystem> FXClass, FVector Location, float Radius)
{
	UNiagaraSystem* FX = FXClass.LoadSynchronous();
	if (!FX)
	{
		return;
	}

	UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(), FX, Location, FRotator::ZeroRotator, FVector(1.f), true, true);

	if (NiagaraComp)
	{
		NiagaraComp->SetVariableFloat(FName(TEXT("Radius")), Radius);

		// 루프 이펙트 자동 제거 — 1.3초 후 Deactivate (bAutoDestroy=true이므로 즉시 파괴)
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
}
