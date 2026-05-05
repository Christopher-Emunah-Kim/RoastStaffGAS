// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/GA_CharacterSkill.h"
#include "RoastStaffGAS.h"
#include "GAS/Tags/RSGameplayTags.h"
#include "Objects/Data/RSCharacterSkillData.h"
#include "Subsystems/SkillManagerSubsystem.h"
#include "Subsystems/PoolingSubsystem.h"
#include "Character/BaseCharacter.h"
#include "Character/Player/RSPlayerController.h"
#include "Data/EnumTypes.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Objects/Projectile/BaseProjectile.h"
#include "Interface/SkillEffectInterface.h"
#include "GAS/Attributes/PlayerAttributeSet.h"
#include "System/LoggingSystem.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
#if WITH_EDITOR
#include "DrawDebugHelpers.h"
#endif


// ============================================================================
// OnAbilityActivated
// ============================================================================

void UGA_CharacterSkill::OnAbilityActivated(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	const URSCharacterSkillData* SkillData = Cast<URSCharacterSkillData>(GetCurrentSourceObject());
	if (!ensureMsgf(SkillData, TEXT("SourceObject가 URSCharacterSkillData가 아님")))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	GET_WORLD_SUBSYSTEM(USkillManagerSubsystem, SkillMgr)

	const FCharacterSkillExecData ExecData = SkillMgr->GetSlotExecData(SkillData->SlotIndex);

	switch (ExecData.TargetingType)
	{
	case ESkillTargetingType::Instant:
		StartSkillWithMontage(ExecData, Handle, ActorInfo, ActivationInfo,
			[this, ExecData, Handle, ActorInfo, ActivationInfo]()
			{
				ResolveEffect(ExecData, Handle, ActorInfo, ActivationInfo, FVector::ZeroVector);
			},
			!bUseRootMotion);
		break;
	case ESkillTargetingType::AimPreview:
		StartSkillWithMontage(ExecData, Handle, ActorInfo, ActivationInfo,
			[this, ExecData, Handle, ActorInfo, ActivationInfo]()
			{
				ResolveTargeting_AimPreview(ExecData, Handle, ActorInfo, ActivationInfo);
			},
			!bUseRootMotion);
		break;
	case ESkillTargetingType::LaunchProjectile:
		StartSkillWithMontage(ExecData, Handle, ActorInfo, ActivationInfo,
			[this, ExecData, Handle, ActorInfo, ActivationInfo]()
			{
				ResolveTargeting_LaunchProjectile(ExecData, Handle, ActorInfo, ActivationInfo);
			},
			!bUseRootMotion);
		break;
	case ESkillTargetingType::ChargeAndRelease:
		KHS_WARN(TEXT("ChargeAndRelease는 DEFERRED — SlotIndex: %d"), SkillData->SlotIndex);
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		break;
	default:
		KHS_WARN(TEXT("처리되지 않은 TargetingType — SlotIndex: %d"), SkillData->SlotIndex);
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		break;
	}
}

// ============================================================================
// StartSkillWithMontage
// ============================================================================

void UGA_CharacterSkill::StartSkillWithMontage(
	const FCharacterSkillExecData& ExecData,
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	TFunction<void()> ExecuteFunc,
	bool bLockMovement)
{
	if (!CastingMontage)
	{
		ExecuteFunc();
		return;
	}

	ABaseCharacter* Instigator = const_cast<ABaseCharacter*>(CachedInstigator.Get());
	if (bLockMovement && Instigator)
	{
		if (UCharacterMovementComponent* MoveComp = Instigator->FindComponentByClass<UCharacterMovementComponent>())
		{
			MoveComp->DisableMovement();
		}
	}

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, NAME_None, CastingMontage, 1.f, NAME_None, true);

	UAbilityTask_WaitGameplayEvent* EventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, RSTags::Event_Montage_HitCheck);

	EventTask->EventReceived.AddDynamic(this, &UGA_CharacterSkill::OnHitCheckReceived);

	MontageTask->OnCompleted.AddDynamic(this, &UGA_CharacterSkill::OnCastingMontageEnded);
	MontageTask->OnBlendOut.AddDynamic(this, &UGA_CharacterSkill::OnCastingMontageEnded);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_CharacterSkill::OnCastingMontageEnded);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_CharacterSkill::OnCastingMontageEnded);

	PendingExecuteFunc = MoveTemp(ExecuteFunc);
	bExecuteFuncCalled = false;

	EventTask->ReadyForActivation();
	MontageTask->ReadyForActivation();
}

void UGA_CharacterSkill::OnHitCheckReceived(FGameplayEventData Payload)
{
	if (bExecuteFuncCalled)
	{
		return;
	}
	bExecuteFuncCalled = true;

	if (PendingExecuteFunc)
	{
		PendingExecuteFunc();
		PendingExecuteFunc = nullptr;
	}
}

void UGA_CharacterSkill::OnCastingMontageEnded()
{
	ABaseCharacter* Instigator = const_cast<ABaseCharacter*>(CachedInstigator.Get());
	if (Instigator)
	{
		if (UCharacterMovementComponent* MoveComp = Instigator->FindComponentByClass<UCharacterMovementComponent>())
		{
			MoveComp->SetMovementMode(EMovementMode::MOVE_Walking);
		}
	}

	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}

// ============================================================================
// ResolveTargeting
// ============================================================================

void UGA_CharacterSkill::ResolveTargeting_AimPreview(
	const FCharacterSkillExecData& ExecData,
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	GET_WORLD_SUBSYSTEM(USkillManagerSubsystem, SkillMgr)
	const FVector TargetLoc = SkillMgr->GetPendingTargetLocation();
	ResolveEffect(ExecData, Handle, ActorInfo, ActivationInfo, TargetLoc);
}

void UGA_CharacterSkill::ResolveTargeting_LaunchProjectile(
	const FCharacterSkillExecData& ExecData,
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	ResolveEffect(ExecData, Handle, ActorInfo, ActivationInfo, FVector::ZeroVector);
}

// ============================================================================
// ResolveEffect
// ============================================================================

void UGA_CharacterSkill::ResolveEffect(
	const FCharacterSkillExecData& ExecData,
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	FVector TargetLocation)
{
	switch (ExecData.EffectType)
	{
	case ESkillEffectType::RadialAoE:
	{
		// Instant + RadialAoE: 에임 방향 기준 중심점 계산
		if (TargetLocation.IsZero() && CachedInstigator)
		{
			const FVector PlayerLoc = CachedInstigator->GetActorLocation();
			FVector AimDir = CachedInstigator->GetActorForwardVector();
			if (const ARSPlayerController* PC = Cast<ARSPlayerController>(GetWorld()->GetFirstPlayerController()))
			{
				const FVector Dir = (PC->GetCachedAimLocation() - PlayerLoc).GetSafeNormal2D();
				if (!Dir.IsNearlyZero())
				{
					AimDir = Dir;
				}
			}
			TargetLocation = PlayerLoc + AimDir * FMath::Max(1.f, ExecData.EffectRadius);
		}
		ExecuteEffect_RadialAoE(ExecData, Handle, ActorInfo, ActivationInfo, TargetLocation);
		break;
	}
	case ESkillEffectType::SelfBuff:
		ExecuteEffect_SelfBuff(ExecData, Handle, ActorInfo, ActivationInfo);
		break;
	case ESkillEffectType::Teleport:
		ExecuteEffect_Teleport(ExecData, Handle, ActorInfo, ActivationInfo, TargetLocation);
		break;
	case ESkillEffectType::SpawnActor:
	{
		// Instant 경로 (TargetLocation 미지정): 캐릭터 전방 300cm 고정 스폰
		// AimPreview 경로는 TargetLocation이 이미 설정됨
		if (TargetLocation.IsZero() && CachedInstigator)
		{
			constexpr float AutomatonSpawnOffset = 300.f;
			TargetLocation = CachedInstigator->GetActorLocation()
				+ CachedInstigator->GetActorForwardVector() * AutomatonSpawnOffset;
		}
		ExecuteEffect_SpawnActor(ExecData, Handle, ActorInfo, ActivationInfo, TargetLocation);
		break;
	}
	case ESkillEffectType::Projectile:
		// BackstepDistance > 0 이면 백스텝샷 전용 경로 — 후방이동+SelfBuff가 EndAbility 담당
		if (ExecData.BackstepDistance > 0.f)
		{
			ExecuteEffect_BackstepShot(ExecData, Handle, ActorInfo, ActivationInfo);
		}
		else
		{
			ExecuteEffect_Projectile(ExecData, Handle, ActorInfo, ActivationInfo);
		}
		break;
		
	default:
		KHS_WARN(TEXT("처리되지 않은 EffectType — SkillID: %s"), *ExecData.SkillID.ToString());
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		break;
	}
}

// ============================================================================
// ExecuteEffect_*
// ============================================================================

void UGA_CharacterSkill::ExecuteEffect_RadialAoE(
	const FCharacterSkillExecData& ExecData,
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	FVector TargetLocation)
{
	if (!CachedInstigator)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	TSubclassOf<UGameplayEffect> LoadedSkillGE = ExecData.SkillGEClass.LoadSynchronous();

	const float Radius = FMath::Max(1.f, ExecData.EffectRadius);

	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(CachedInstigator.Get());

	TRACE_BOOKMARK(TEXT("SkillEffect_AoE_Overlap"))
	GetWorld()->OverlapMultiByChannel(Overlaps, TargetLocation, FQuat::Identity, ECC_Pawn,
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

		FHitResult CenterHit;
		CenterHit.ImpactPoint = TargetLocation;
		Context.AddHitResult(CenterHit, true);

		FGameplayEffectSpecHandle Spec = OwnerASC->MakeOutgoingSpec(LoadedSkillGE, 1, Context);
		if (!Spec.IsValid())
		{
			continue;
		}

		Spec.Data->SetByCallerTagMagnitudes.Add(RSTags::Data_WeaponBaseDamage, GetSkillDamageAmount(ExecData));
		OwnerASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
	}

	// FX
	const FVector PlayerLoc = CachedInstigator->GetActorLocation();
	FVector AimDir = (TargetLocation - PlayerLoc).GetSafeNormal2D();
	if (AimDir.IsNearlyZero())
	{
		AimDir = CachedInstigator->GetActorForwardVector();
	}
	FRotator FXRotation = AimDir.Rotation();
	FXRotation.Yaw += 180.f;
	SpawnSkillFX(ExecData.SkillFX, TargetLocation, Radius, ExecData.ElementTag, 0.f, FXRotation);

	KHS_INFO(TEXT("RadialAoE 발동 — SkillID: %s | 반경: %.0f"), *ExecData.SkillID.ToString(), Radius);

	if (!CastingMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UGA_CharacterSkill::ExecuteEffect_SelfBuff(
	const FCharacterSkillExecData& ExecData,
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	TSubclassOf<UGameplayEffect> LoadedSkillGE = ExecData.SkillGEClass.LoadSynchronous();

	UAbilitySystemComponent* OwnerASC = GetOwnerASC();

	FGameplayEffectContextHandle Context = OwnerASC->MakeEffectContext();
	Context.AddInstigator(const_cast<ABaseCharacter*>(CachedInstigator.Get()), const_cast<ABaseCharacter*>(CachedInstigator.Get()));
	FGameplayEffectSpecHandle Spec = OwnerASC->MakeOutgoingSpec(LoadedSkillGE, 1, Context);

	if (!Spec.IsValid())
	{
		KHS_WARN(TEXT("GE 스펙 생성 실패 — SkillID: %s"), *ExecData.SkillID.ToString());
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	OwnerASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());

	if (CachedInstigator)
	{
		TRACE_BOOKMARK(TEXT("SkillFX_BuffAura_Spawn"))
		UNiagaraSystem* FX = ExecData.SkillFX.LoadSynchronous();
		if (FX)
		{
			UNiagaraComponent* BuffFX = UNiagaraFunctionLibrary::SpawnSystemAttached(
				FX,
				CachedInstigator->GetMesh(),
				NAME_None,
				FVector::ZeroVector,
				FRotator::ZeroRotator,
				EAttachLocation::SnapToTarget,
				true
			);

			if (BuffFX)
			{
				BuffFX->SetVariableFloat(FName(TEXT("Radius")), ExecData.EffectRadius);

				TWeakObjectPtr<UNiagaraComponent> WeakFX(BuffFX);
				FTimerHandle FXHandle;
				GetWorld()->GetTimerManager().SetTimer(FXHandle, [WeakFX]()
				{
					if (WeakFX.IsValid())
					{
						WeakFX->Deactivate();
					}
				}, ExecData.Duration, false);
			}
		}
	}

	KHS_INFO(TEXT("SelfBuff 발동 — SkillID: %s | Duration: %.1fs"), *ExecData.SkillID.ToString(), ExecData.Duration);

	if (!CastingMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UGA_CharacterSkill::ExecuteEffect_Teleport(
	const FCharacterSkillExecData& ExecData,
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	FVector TargetLocation)
{
	if (!CachedInstigator)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const float Radius = FMath::Max(1.f, ExecData.EffectRadius);
	const FVector DepartureLoc = CachedInstigator->GetActorLocation();

	FRotator FXRotation = FRotator::ZeroRotator;
	if (const APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		FXRotation = PC->GetControlRotation();
		FXRotation.Pitch = 0.f;
		FXRotation.Roll  = 0.f;
	}

	// 출발지 FX
	if (FXActorClass)
	{
		GetWorld()->SpawnActor<AActor>(FXActorClass, DepartureLoc, FRotator::ZeroRotator);
	}
	else
	{
		SpawnSkillFX(ExecData.SkillFX, DepartureLoc, Radius, ExecData.ElementTag, DESTROY_FX_DELAY, FXRotation);
	}

	// 텔레포트
	ABaseCharacter* MutableInstigator = const_cast<ABaseCharacter*>(CachedInstigator.Get());
	MutableInstigator->SetActorLocation(TargetLocation, false, nullptr, ETeleportType::TeleportPhysics);

	// 도착지 FX
	if (FXActorClass)
	{
		GetWorld()->SpawnActor<AActor>(FXActorClass, TargetLocation, FRotator::ZeroRotator);
	}
	else
	{
		SpawnSkillFX(ExecData.SkillFX, TargetLocation, Radius, ExecData.ElementTag, DESTROY_FX_DELAY, FXRotation);
	}

	KHS_INFO(TEXT("Teleport 발동 — SkillID: %s | 위치: %s"), *ExecData.SkillID.ToString(), *TargetLocation.ToString());

	if (!CastingMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UGA_CharacterSkill::ExecuteEffect_SpawnActor(
	const FCharacterSkillExecData& ExecData,
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	FVector TargetLocation)
{
	TSubclassOf<AActor> EffectClass;
	if (!LoadRequiredClass(ExecData.EffectActorClass, EffectClass, ExecData.SkillID))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// TargetLocation이 ZeroVector이면 (Instant 경로) 마우스 에임 위치 사용
	if (TargetLocation.IsZero())
	{
		if (const ARSPlayerController* PC = Cast<ARSPlayerController>(GetWorld()->GetFirstPlayerController()))
		{
			TargetLocation = PC->GetCachedAimLocation();
		}
		else if (CachedInstigator)
		{
			TargetLocation = CachedInstigator->GetActorLocation();
			KHS_WARN(TEXT("ARSPlayerController 캐스트 실패 — 시전자 위치 폴백. SkillID: %s"), *ExecData.SkillID.ToString());
		}
	}

	GET_WORLD_SUBSYSTEM(UPoolingSubsystem, PoolSub)
	AActor* EffectActor = PoolSub->SpawnPooledActor<AActor>(
		EffectClass, FTransform(FRotator::ZeroRotator, TargetLocation));

	if (!EffectActor)
	{
		KHS_WARN(TEXT("SpawnActor 스폰 실패 — SkillID: %s"), *ExecData.SkillID.ToString());
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ISkillEffectInterface* EffectInterface = Cast<ISkillEffectInterface>(EffectActor);
	if (!EffectInterface)
	{
		KHS_WARN(TEXT("EffectActor가 ISkillEffectInterface를 구현하지 않음 — SkillID: %s"), *ExecData.SkillID.ToString());
		PoolSub->ReturnToPool(EffectActor);
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	FSkillEffectInitData InitData;
	InitData.InstigatorASC     = GetOwnerASC();
	InitData.SkillGEClass      = ExecData.SkillGEClass.LoadSynchronous();
	InitData.StatusGEClass     = ExecData.StatusGEClass.LoadSynchronous();
	InitData.Amount            = GetSkillDamageAmount(ExecData);
	InitData.EffectRadius      = ExecData.EffectRadius;
	InitData.Duration          = ExecData.Duration;
	InitData.SkillFX           = ExecData.SkillFX;
	InitData.ElementColor      = ResolveElementColor(ExecData.ElementTag);
	InitData.SpawnCount        = ExecData.SpawnCount;
	if (CachedInstigator)
	{
		InitData.InstigatorForward = CachedInstigator->GetActorForwardVector();
	}
	EffectInterface->InitEffect(InitData);

	KHS_INFO(TEXT("SpawnActor 발동 — SkillID: %s | 위치: %s"), *ExecData.SkillID.ToString(), *TargetLocation.ToString());

	// SpawnActor는 EffectActor가 독립 수명 관리 — GA 즉시 종료
	if (!CastingMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UGA_CharacterSkill::ExecuteEffect_Projectile(
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

	// 애로우레인: Circle 패턴 + ZOffset > 0 조합 감지 → 전용 경로
	if (ExecData.SpawnPattern == ESkillSpawnPattern::Circle && ExecData.ZOffset > 0.f)
	{
		ExecuteEffect_ArrowRain(ExecData, Handle, ActorInfo, ActivationInfo, LoadedClass);
		return;
	}

	const int32 TotalCount = FMath::Max(1, ExecData.SpawnCount);

	if (TotalCount <= 1 || ExecData.SpawnPattern != ESkillSpawnPattern::Burst)
	{
		// 단발 or Spread/Circle — 즉시 발사 후 종료
		FProjectileInitData InitData = BuildProjectileInitData(ExecData, LoadedClass);
		SpawnProjectiles(LoadedClass, InitData);
		SpawnSkillFX(ExecData.SkillFX, CachedInstigator ? CachedInstigator->GetActorLocation() : FVector::ZeroVector, ExecData.EffectRadius);

		if (!CastingMontage)
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		}
		return;
	}

	// Burst — 첫 발 즉시, 이후 타이머
	ActiveProjClass    = LoadedClass;
	CachedProjExecData = ExecData;
	RemainingFireCount = TotalCount - 1;

	FProjectileInitData InitData = BuildProjectileInitData(ExecData, LoadedClass);
	SpawnProjectiles(LoadedClass, InitData);
	SpawnSkillFX(ExecData.SkillFX, CachedInstigator ? CachedInstigator->GetActorLocation() : FVector::ZeroVector, ExecData.EffectRadius);

	TWeakObjectPtr<UGA_CharacterSkill> WeakThis(this);
	GetWorld()->GetTimerManager().SetTimer(
		MultiFireTimerHandle,
		[WeakThis]()
		{
			if (!WeakThis.IsValid())
			{
				return;
			}

			FProjectileInitData Data = WeakThis->BuildProjectileInitData(WeakThis->CachedProjExecData, WeakThis->ActiveProjClass);
			
			WeakThis->SpawnProjectiles(WeakThis->ActiveProjClass, Data);
			WeakThis->SpawnSkillFX(WeakThis->CachedProjExecData.SkillFX,
				IsValid(WeakThis->CachedInstigator) ? WeakThis->CachedInstigator->GetActorLocation() : FVector::ZeroVector,
				WeakThis->CachedProjExecData.EffectRadius);
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

	KHS_INFO(TEXT("Burst 발사 시작 — SkillID: %s | %d발 × %.2fs"), *ExecData.SkillID.ToString(), TotalCount, ExecData.FireInterval);
}

// ============================================================================
// 헬퍼
// ============================================================================

FProjectileInitData UGA_CharacterSkill::BuildProjectileInitData(const FCharacterSkillExecData& ExecData, TSubclassOf<ABaseProjectile> /*ProjClass*/) const
{
	FProjectileInitData InitData;
	InitData.SkillID       = ExecData.SkillID;
	InitData.DamageGEClass = ExecData.SkillGEClass.LoadSynchronous();
	InitData.InstigatorASC = GetOwnerASC();
	InitData.Amount        = GetSkillDamageAmount(ExecData);
	InitData.Speed         = ExecData.ProjectileSpeed;
	InitData.Lifetime      = ExecData.ProjectileRange / FMath::Max(1.f, ExecData.ProjectileSpeed);
	InitData.SpawnCount    = 1;
	InitData.SpreadAngle   = 0.f;

	// ProjectileMoveType → 기존 EMoveType 매핑 (BaseProjectile 인터페이스 호환)
	switch (ExecData.ProjectileMoveType)
	{
	case EProjectileMoveType::Linear:
		InitData.MoveType = EMoveType::LINEAR;
		InitData.HitType  = EHitType::SINGLE;
		break;
	case EProjectileMoveType::Pierce:
		InitData.MoveType    = EMoveType::LINEAR;
		InitData.HitType     = EHitType::PIERCE;
		InitData.PierceCount = FMath::Max(1, ExecData.PierceCount);
		InitData.DamageDecay = ExecData.DamageDecay;
		break;
	case EProjectileMoveType::Homing:
		InitData.MoveType = EMoveType::HOMING;
		InitData.HitType  = EHitType::SINGLE;
		break;
	case EProjectileMoveType::HomingBounce:
		InitData.MoveType = EMoveType::HOMING_BOUNCE;
		InitData.HitType  = EHitType::SINGLE;
		// 첫 타겟 탐색
		if (CachedInstigator)
		{
			constexpr float InitSearchRadius = 2000.f;
			const FVector Origin  = CachedInstigator->GetActorLocation();
			const FVector Forward = CachedInstigator->GetActorForwardVector();

			TArray<FOverlapResult> Overlaps;
			FCollisionQueryParams QueryParams;
			QueryParams.AddIgnoredActor(CachedInstigator.Get());
			GetWorld()->OverlapMultiByChannel(Overlaps, Origin, FQuat::Identity,
				ECC_Pawn, FCollisionShape::MakeSphere(InitSearchRadius), QueryParams);

			float MinDist = FLT_MAX;
			for (const FOverlapResult& Overlap : Overlaps)
			{
				AActor* Candidate = Overlap.GetActor();
				if (!Candidate)
				{
					continue;
				}
				UAbilitySystemComponent* CandASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Candidate);
				if (!CandASC || !CandASC->HasMatchingGameplayTag(RSTags::Team_Enemy))
				{
					continue;
				}
				const FVector ToCandidate = (Candidate->GetActorLocation() - Origin).GetSafeNormal();
				if (FVector::DotProduct(Forward, ToCandidate) <= 0.f)
				{
					continue;
				}
				const float Dist = FVector::Dist(Origin, Candidate->GetActorLocation());
				if (Dist < MinDist)
				{
					MinDist = Dist;
					InitData.HomingTarget = Candidate->GetRootComponent();
				}
			}
			if (!InitData.HomingTarget.IsValid())
			{
				KHS_WARN(TEXT("HOMING_BOUNCE 첫 타겟 없음 — 직선 발사. SkillID: %s"), *ExecData.SkillID.ToString());
			}
		}
		break;
	case EProjectileMoveType::Explode:
		InitData.MoveType = EMoveType::LINEAR;
		InitData.HitType  = EHitType::AREA;
		break;
		
	default:
		InitData.MoveType = EMoveType::LINEAR;
		InitData.HitType  = EHitType::SINGLE;
		break;
	}

	return InitData;
}

FLinearColor UGA_CharacterSkill::ResolveElementColor(FGameplayTag ElementTag)
{
	if (ElementTag == RSTags::Element_Fire)
	{
		return FLinearColor(1.f, 0.3f, 0.f, 1.f);
	}
	else if (ElementTag == RSTags::Element_Ice)
	{
		return FLinearColor(0.3f, 0.8f, 1.f, 1.f);
	}
	else if (ElementTag == RSTags::Element_Thunder)
	{
		return FLinearColor(1.f, 1.f, 0.f, 1.f);
	}
	else if (ElementTag == RSTags::Element_Ancient)
	{
		return FLinearColor(0.5f, 0.05f, 0.4f, 1.f);
	}
	return FLinearColor::White;
}

void UGA_CharacterSkill::SpawnSkillFX(TSoftObjectPtr<UNiagaraSystem> FXClass, FVector Location, float Radius,
	FGameplayTag ElementTag, float FXLifetime, FRotator Rotation)
{
	TRACE_BOOKMARK(TEXT("SkillFX_SyncLoad"))
	UNiagaraSystem* FX = FXClass.LoadSynchronous();
	if (!FX)
	{
		return;
	}

	TRACE_BOOKMARK(TEXT("SkillFX_NiagaraSpawn"))
	UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(), FX, Location, Rotation, FVector(1.f), true, true);

	if (!NiagaraComp)
	{
		return;
	}

	NiagaraComp->SetVariableFloat(FName(TEXT("Radius")), Radius);
	NiagaraComp->SetVariableLinearColor(FName(TEXT("ElementColor")), ResolveElementColor(ElementTag));

	const float ActualLifetime = FXLifetime > 0.f ? FXLifetime : DESTROY_FX_DELAY;

	TWeakObjectPtr<UNiagaraComponent> WeakComp(NiagaraComp);
	FTimerHandle FXLifetimeHandle;
	GetWorld()->GetTimerManager().SetTimer(FXLifetimeHandle, [WeakComp]()
	{
		if (WeakComp.IsValid())
		{
			WeakComp->Deactivate();
		}
	}, ActualLifetime, false);
}

float UGA_CharacterSkill::GetSkillDamageAmount(const FCharacterSkillExecData& ExecData) const
{
	UAbilitySystemComponent* OwnerASC = GetOwnerASC();
	if (!OwnerASC)
	{
		KHS_WARN(TEXT("GetSkillDamageAmount: OwnerASC 없음 — 0 반환"));
		return 0.f;
	}

	bool bFound = false;
	const float ATK = OwnerASC->GetGameplayAttributeValue(
		UPlayerAttributeSet::GetATKAttribute(), bFound);

	if (!bFound)
	{
		KHS_WARN(TEXT("GetSkillDamageAmount: ATK 어트리뷰트 없음 — 0 반환"));
		return 0.f;
	}

	return ATK * ExecData.DamageMultiplier;
}

// ── 백스텝샷 ─────────────────────────────────────────────────────────────────

AActor* UGA_CharacterSkill::FindNearestEnemy(FVector PlayerLoc, float SearchRadius) const
{
	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams QueryParams;
	if (CachedInstigator)
	{
		QueryParams.AddIgnoredActor(CachedInstigator.Get());
	}

	GetWorld()->OverlapMultiByChannel(Overlaps, PlayerLoc, FQuat::Identity,
		ECC_Pawn, FCollisionShape::MakeSphere(FMath::Max(1.f, SearchRadius)), QueryParams);

	AActor* Nearest = nullptr;
	float MinDistSq = FLT_MAX;

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Candidate = Overlap.GetActor();
		if (!Candidate)
		{
			continue;
		}

		UAbilitySystemComponent* CandASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Candidate);
		if (!CandASC || !CandASC->HasMatchingGameplayTag(RSTags::Team_Enemy))
		{
			continue;
		}

		const float DistSq = FVector::DistSquared(PlayerLoc, Candidate->GetActorLocation());
		if (DistSq < MinDistSq)
		{
			MinDistSq = DistSq;
			Nearest = Candidate;
		}
	}

	return Nearest;
}

void UGA_CharacterSkill::ExecuteEffect_BackstepShot(
	const FCharacterSkillExecData& ExecData,
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	if (!CachedInstigator)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const FVector PlayerLoc = CachedInstigator->GetActorLocation();

	// 1. 마우스 에임 방향 → 발사 방향 / 백스텝 방향 결정
	FVector FireDir = CachedInstigator->GetActorForwardVector();
	if (const ARSPlayerController* PC = Cast<ARSPlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		const FVector AimDir = (PC->GetCachedAimLocation() - PlayerLoc).GetSafeNormal2D();
		if (!AimDir.IsNearlyZero())
		{
			FireDir = AimDir;
		}
	}
	const FVector BackstepDir = -FireDir;

	// 2. 투사체 발사 (적 방향 직접 스폰)
	TSubclassOf<ABaseProjectile> LoadedClass = ExecData.ProjectileClass.LoadSynchronous();
	if (LoadedClass)
	{
		FProjectileInitData InitData = BuildProjectileInitData(ExecData, LoadedClass);
		const FVector SpawnLoc = PlayerLoc + FireDir * SPAWN_OFFSET;
		const FRotator SpawnRot = FireDir.ToOrientationRotator();

		GET_WORLD_SUBSYSTEM(UPoolingSubsystem, PoolSys)
		ABaseProjectile* Projectile = PoolSys->SpawnPooledActor<ABaseProjectile>(
			LoadedClass, FTransform(SpawnRot, SpawnLoc));

		if (Projectile)
		{
			Projectile->SetOwner(const_cast<ABaseCharacter*>(CachedInstigator.Get()));
			Projectile->SetInstigator(const_cast<ABaseCharacter*>(CachedInstigator.Get()));
			Projectile->InitProjectile(InitData);
		}
		else
		{
			KHS_WARN(TEXT("투사체 스폰 실패. SkillID: %s"), *ExecData.SkillID.ToString());
		}
	}
	else
	{
		KHS_WARN(TEXT("ProjectileClass 로드 실패. SkillID: %s"), *ExecData.SkillID.ToString());
	}

	// 3. 후방 LaunchCharacter + 즉시 SelfBuff
	ABaseCharacter* MutableInstigator = const_cast<ABaseCharacter*>(CachedInstigator.Get());

	// DisableMovement 상태에서는 LaunchCharacter가 무시됨 — 발사 직전 복원
	if (UCharacterMovementComponent* MoveComp = MutableInstigator->FindComponentByClass<UCharacterMovementComponent>())
	{
		MoveComp->SetMovementMode(MOVE_Falling);
	}

	// BackstepDistance를 BackstepDuration 안에 주파할 속도 계산
	const FVector LaunchVelocity = BackstepDir * (ExecData.BackstepDistance / BACKSTEP_DURATION);
	MutableInstigator->LaunchCharacter(LaunchVelocity, /*bXYOverride=*/true, /*bZOverride=*/true);

	ExecuteEffect_SelfBuff(ExecData, Handle, ActorInfo, ActivationInfo);

	KHS_INFO(TEXT("BackstepShot 발동 — SkillID: %s | 후방거리: %.0f"), *ExecData.SkillID.ToString(), ExecData.BackstepDistance);
}

// ── 애로우레인 ────────────────────────────────────────────────────────────────

void UGA_CharacterSkill::ExecuteEffect_ArrowRain(
	const FCharacterSkillExecData& ExecData,
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	TSubclassOf<ABaseProjectile> LoadedClass)
{
	if (!CachedInstigator)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 발사 시점 캐릭터 방향 스냅 — 이후 캐릭터 회전과 무관하게 고정
	const FVector Forward = CachedInstigator->GetActorForwardVector();
	const FVector Right   = CachedInstigator->GetActorRightVector();

	// 스폰 기준점: 전방 400cm + 공중 ZOffset 높이
	const FVector Origin = CachedInstigator->GetActorLocation()
		+ Forward * -200.f
		+ FVector(0.f, 0.f, ExecData.ZOffset);

	// 전방 기준 60도 하향 발사 방향 (Right 축 기준 회전 → 캐릭터 전방 추종)
	const FVector FireDir = FVector::DownVector.RotateAngleAxis(-60.f, Right).GetSafeNormal();
	const FRotator FireRotation = FireDir.ToOrientationRotator();

	const int32 Count = FMath::Max(1, ExecData.SpawnCount);
	const float Radius = FMath::Max(1.f, ExecData.EffectRadius);

	FProjectileInitData InitData = BuildProjectileInitData(ExecData, LoadedClass);
	// StatusGEClass 이속감소 GE — BaseProjectile::ApplyMultipleEffectsToTarget이 OnHit 시 자동 Apply
	InitData.StatusGEClass = ExecData.StatusGEClass.LoadSynchronous();
	InitData.SpawnCount    = 1;

	GET_WORLD_SUBSYSTEM(UPoolingSubsystem, PoolSys)

	for (int32 i = 0; i < Count; ++i)
	{
		// 반경 내 랜덤 XY 오프셋 — 각 화살 독립
		const FVector2D RandOffset = FMath::RandPointInCircle(Radius);
		const FVector SpawnLoc = Origin + FVector(RandOffset.X, RandOffset.Y, 0.f);

		ABaseProjectile* Projectile = PoolSys->SpawnPooledActor<ABaseProjectile>(
			LoadedClass, FTransform(FireRotation, SpawnLoc));

		if (!Projectile)
		{
			KHS_WARN(TEXT("ArrowRain 투사체 스폰 실패 — 인덱스: %d"), i);
			continue;
		}

		Projectile->SetOwner(const_cast<ABaseCharacter*>(CachedInstigator.Get()));
		Projectile->SetInstigator(const_cast<ABaseCharacter*>(CachedInstigator.Get()));
		Projectile->InitProjectile(InitData);
	}

	SpawnSkillFX(ExecData.SkillFX, CachedInstigator->GetActorLocation(), Radius, ExecData.ElementTag);

	KHS_INFO(TEXT("ArrowRain 발동 — SkillID: %s | %d발 | 반경: %.0f"), *ExecData.SkillID.ToString(), Count, Radius);

	if (!CastingMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

