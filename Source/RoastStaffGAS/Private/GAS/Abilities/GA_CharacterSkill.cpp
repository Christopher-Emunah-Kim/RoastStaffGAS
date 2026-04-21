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
#include "Objects/GroundEffect/GroundEffectActor.h"
#include "System/LoggingSystem.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GameFramework/CharacterMovementComponent.h"
#if WITH_EDITOR
#include "DrawDebugHelpers.h"
#endif


void UGA_CharacterSkill::OnAbilityActivated(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* ActorInfo,const FGameplayAbilityActivationInfo ActivationInfo)
{
	const URSCharacterSkillData* SkillData = Cast<URSCharacterSkillData>(GetCurrentSourceObject());
	if (!ensureMsgf(SkillData, TEXT("SourceObject가 URSCharacterSkillData가 아님")))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	GET_WORLD_SUBSYSTEM(USkillManagerSubsystem, SkillMgr)

	const FCharacterSkillExecData ExecData = SkillMgr->GetSlotExecData(SkillData->SlotIndex);

	switch (ExecData.ActivationType)
	{
	case ESkillActivationType::InstantAoE:
		StartSkillWithMontage(ExecData, Handle, ActorInfo, ActivationInfo,
			[this, ExecData, Handle, ActorInfo, ActivationInfo]()
			{
				ExecuteInstantAoE(ExecData, Handle, ActorInfo, ActivationInfo);
			});
		break;
	case ESkillActivationType::SelfBuff:
		StartSkillWithMontage(ExecData, Handle, ActorInfo, ActivationInfo,
			[this, ExecData, Handle, ActorInfo, ActivationInfo]()
			{
				ExecuteSelfBuff(ExecData, Handle, ActorInfo, ActivationInfo);
			});
		break;
	case ESkillActivationType::SpawnPreview:
		StartSkillWithMontage(ExecData, Handle, ActorInfo, ActivationInfo,
			[this, ExecData, Handle, ActorInfo, ActivationInfo]()
			{
				ExecuteSpawnPreview(ExecData, Handle, ActorInfo, ActivationInfo);
			});
		break;
	case ESkillActivationType::ProjectileSpawn:
		StartSkillWithMontage(ExecData, Handle, ActorInfo, ActivationInfo,
			[this, ExecData, Handle, ActorInfo, ActivationInfo]()
			{
				ExecuteProjectileSpawn(ExecData, Handle, ActorInfo, ActivationInfo);
			});
		break;
	case ESkillActivationType::GroundEffect:
		StartSkillWithMontage(ExecData, Handle, ActorInfo, ActivationInfo,
			[this, ExecData, Handle, ActorInfo, ActivationInfo]()
			{
				ExecuteGroundEffect(ExecData, Handle, ActorInfo, ActivationInfo);
			});
		break;
	default:
		KHS_WARN(TEXT("처리되지 않은 ActivationType — SlotIndex: %d"), SkillData->SlotIndex);
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		break;
	}
}

void UGA_CharacterSkill::StartSkillWithMontage(
	const FCharacterSkillExecData& ExecData,
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	TFunction<void()> ExecuteFunc)
{
	// 몽타주 미할당 시 즉시 발동 (기존 동작 유지)
	if (!CastingMontage)
	{
		ExecuteFunc();
		return;
	}

	// 이동 잠금
	ABaseCharacter* Instigator = const_cast<ABaseCharacter*>(CachedInstigator.Get());
	if (Instigator)
	{
		if (UCharacterMovementComponent* MoveComp = Instigator->FindComponentByClass<UCharacterMovementComponent>())
		{
			MoveComp->DisableMovement();
		}
	}

	// 몽타주 재생 태스크
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, NAME_None, CastingMontage, 1.f, NAME_None, true);

	// HitCheck 노티파이 대기 태스크
	UAbilityTask_WaitGameplayEvent* EventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, RSTags::Event_Montage_HitCheck);

	// HitCheck 노티파이 수신 → 실제 효과 발동
	EventTask->EventReceived.AddDynamic(this, &UGA_CharacterSkill::OnHitCheckReceived);

	// 몽타주 완료 → 이동 복원 + EndAbility
	// 람다 캡처 불가(AddDynamic) → OnMontageEnded 멤버함수로 위임
	MontageTask->OnCompleted.AddDynamic(this, &UGA_CharacterSkill::OnCastingMontageEnded);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_CharacterSkill::OnCastingMontageEnded);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_CharacterSkill::OnCastingMontageEnded);

	// ExecuteFunc를 멤버 변수에 캐싱 (노티파이 수신 시 호출)
	PendingExecuteFunc = MoveTemp(ExecuteFunc);
	bExecuteFuncCalled = false;

	EventTask->ReadyForActivation();
	MontageTask->ReadyForActivation();

	KHS_INFO(TEXT("스킬 몽타주 시작 — SkillID: %s"), *ExecData.SkillID.ToString());
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
	// 이동 복원
	ABaseCharacter* Instigator = const_cast<ABaseCharacter*>(CachedInstigator.Get());
	if (Instigator)
	{
		if (UCharacterMovementComponent* MoveComp = Instigator->FindComponentByClass<UCharacterMovementComponent>())
		{
			MoveComp->SetMovementMode(EMovementMode::MOVE_Walking);
		}
	}

	// 몽타주 없이 즉시 발동한 경우는 이 콜백이 오지 않으므로 EndAbility는 각 Execute에서 처리
	// 몽타주 있는 경우: 효과가 HitCheck에서 이미 발동됐으므로 여기서 GA 종료
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
	KHS_INFO(TEXT("스킬 몽타주 종료 — 이동 복원 + EndAbility"));
}

void UGA_CharacterSkill::ExecuteInstantAoE(	const FCharacterSkillExecData& ExecData, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,	const FGameplayAbilityActivationInfo ActivationInfo)
{
	if (!CachedInstigator)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const FVector PlayerLoc = CachedInstigator->GetActorLocation();
	const float Radius      = FMath::Max(1.f, ExecData.LevelData.EffectRadius);

	// 플레이어 → 마우스 방향 (수평면)
	FVector AimDir = CachedInstigator->GetActorForwardVector();
	if (const ARSPlayerController* PC = Cast<ARSPlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		const FVector Dir = (PC->GetCachedAimLocation() - PlayerLoc).GetSafeNormal2D();
		if (!Dir.IsNearlyZero())
		{
			AimDir = Dir;
		}
	}

	// 데미지/FX 중심점: 플레이어에서 에임 방향으로 Radius만큼 전진
	// → 플레이어가 원주 위에 위치, 원이 마우스 방향으로 뻗어나감
	const FVector Center = PlayerLoc + AimDir * Radius;

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

		// Center를 HitResult ImpactPoint로 주입 → EnemyAttributeSet에서 넉백 방향 계산에 사용
		FHitResult CenterHit;
		CenterHit.ImpactPoint = Center;
		Context.AddHitResult(CenterHit, true);

		FGameplayEffectSpecHandle Spec = OwnerASC->MakeOutgoingSpec(SkillGEClass, 1, Context);
		if (!Spec.IsValid())
		{
			continue;
		}

		Spec.Data->SetByCallerTagMagnitudes.Add(RSTags::Data_WeaponBaseDamage, 10.f * ExecData.LevelData.DamageMultiplier);

		OwnerASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
	}

	// FX: Center에서 플레이어 → 마우스 방향으로 스폰
	// FX는 플레이어 → 마우스 방향으로 재생 — AimDir 기준 180도 반전
	FRotator FXRotation = AimDir.Rotation();
	FXRotation.Yaw += 180.f;
	SpawnSkillFX(ExecData.LevelData.FXClass, Center, Radius, ExecData.ElementTag, 0.f, FXRotation);

#if WITH_EDITOR
	// 실제 충돌 판정 구체 시각화 — FX 크기와 비교용 (에디터 전용)
	DrawDebugSphere(GetWorld(), Center, Radius, 24, FColor::Red, false, 0.2f);
	DrawDebugLine(GetWorld(), PlayerLoc, Center, FColor::Yellow, false, 0.2f);
#endif

	KHS_INFO(TEXT("InstantAoE 발동 — SkillID: %s | 반경: %.0f"), *ExecData.SkillID.ToString(), Radius);

	// 몽타주 없이 즉시 발동한 경우만 여기서 종료 — 몽타주 있는 경우 OnCastingMontageEnded에서 종료
	if (!CastingMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
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

	// 버프 오라 FX — 캐릭터 메시에 Attach하여 이동 시 따라다니게 함
	if (CachedInstigator)
	{
		UNiagaraSystem* FX = ExecData.LevelData.FXClass.LoadSynchronous();
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
				BuffFX->SetVariableFloat(FName(TEXT("Radius")), ExecData.LevelData.EffectRadius);

				// Duration 후 오라 FX 비활성화
				TWeakObjectPtr<UNiagaraComponent> WeakFX(BuffFX);
				FTimerHandle FXHandle;
				GetWorld()->GetTimerManager().SetTimer(FXHandle, [WeakFX]()
				{
					if (WeakFX.IsValid())
					{
						WeakFX->Deactivate();
					}
				}, ExecData.LevelData.Duration, false);
			}
		}
	}

	KHS_INFO(TEXT("SelfBuff 발동 — SkillID: %s | Duration: %.1fs"), *ExecData.SkillID.ToString(), ExecData.LevelData.Duration);

	if (!CastingMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UGA_CharacterSkill::ExecuteSpawnPreview(const FCharacterSkillExecData& ExecData, const FGameplayAbilitySpecHandle Handle,	const FGameplayAbilityActorInfo* ActorInfo,	const FGameplayAbilityActivationInfo ActivationInfo)
{
	GET_WORLD_SUBSYSTEM(USkillManagerSubsystem, SkillMgr)
	const FVector TargetLoc = SkillMgr->GetPendingTargetLocation();
	const float Radius      = FMath::Max(1.f, ExecData.LevelData.EffectRadius);

	// AoE 피해 — SkillGEClass 있을 때만 적용 (텔레포트 전용 스킬은 SkillGEClass 없음)
	if (SkillGEClass)
	{
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
	}

	// 장판 소환 — GroundEffectActorClass 있을 때만 (SpawnPreview 확정 위치에 즉시 배치)
	if (ExecData.GroundEffectActorClass)
	{
		GET_WORLD_SUBSYSTEM(UPoolingSubsystem, PoolSub)
		TSubclassOf<AGroundEffectActor> GroundClass;
		if (LoadRequiredClass(ExecData.GroundEffectActorClass, GroundClass, ExecData.SkillID))
		{
			AGroundEffectActor* GroundActor = PoolSub->SpawnPooledActor<AGroundEffectActor>(
				GroundClass, FTransform(FRotator::ZeroRotator, TargetLoc));

			if (GroundActor)
			{
				const float FinalAmount = ExecData.Amount * ExecData.LevelData.DamageMultiplier;
				GroundActor->InitGroundEffect(
					GetOwnerASC(),
					ExecData.LevelData.Duration,
					SkillGEClass,
					ExecData.LevelData.EffectRadius,
					ExecData.LevelData.FXClass,
					FinalAmount);
			}
			else
			{
				KHS_WARN(TEXT("SpawnPreview GroundEffect 스폰 실패 — SkillID: %s"), *ExecData.SkillID.ToString());
			}
		}
	}

	// 텔레포트 분기
	if (bTeleportOnConfirm && CachedInstigator)
	{
		// 출발지 FX
		const FVector DepartureLoc = CachedInstigator->GetActorLocation();
		FRotator FXRotation = FRotator::ZeroRotator;
		if (const APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			FXRotation = PC->GetControlRotation();
			FXRotation.Pitch = 0.f;
			FXRotation.Roll  = 0.f;
		}

		if (FXActorClass)
		{
			GetWorld()->SpawnActor<AActor>(FXActorClass, DepartureLoc, FRotator::ZeroRotator);
		}
		else
		{
			SpawnSkillFX(ExecData.LevelData.FXClass, DepartureLoc, Radius, ExecData.ElementTag, DESTROY_FX_DELAY, FXRotation);
		}

		// 텔레포트
		ABaseCharacter* MutableInstigator = const_cast<ABaseCharacter*>(CachedInstigator.Get());
		MutableInstigator->SetActorLocation(TargetLoc, false, nullptr, ETeleportType::TeleportPhysics);

		// 도착지 FX
		if (FXActorClass)
		{
			GetWorld()->SpawnActor<AActor>(FXActorClass, TargetLoc, FRotator::ZeroRotator);
		}
		else
		{
			SpawnSkillFX(ExecData.LevelData.FXClass, TargetLoc, Radius, ExecData.ElementTag, DESTROY_FX_DELAY, FXRotation);
		}
	}
	else if (!ExecData.GroundEffectActorClass)
	{
		// 텔레포트 없고 장판도 없는 경우 — 목표 위치 FX만 재생
		if (FXActorClass)
		{
			GetWorld()->SpawnActor<AActor>(FXActorClass, TargetLoc, FRotator::ZeroRotator);
		}
		else
		{
			SpawnSkillFX(ExecData.LevelData.FXClass, TargetLoc, Radius, ExecData.ElementTag, DESTROY_FX_DELAY);
		}
	}
	// GroundEffectActorClass 있는 경우 FX는 GroundEffectActor 내부에서 처리

	KHS_INFO(TEXT("SpawnPreview 발동 — SkillID: %s | 위치: %s | Teleport: %s"),
		*ExecData.SkillID.ToString(), *TargetLoc.ToString(), bTeleportOnConfirm ? TEXT("true") : TEXT("false"));

	if (!CastingMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
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
		// 단발 — 즉시 발사 후 종료 (몽타주 없는 경우만)
		FireOneProjectile(LoadedClass, ExecData);
		if (!CastingMontage)
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		}
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

	// HOMING_BOUNCE: 시전자 전방 반구 내 가장 가까운 적을 첫 타겟으로 설정
	if (ExecData.MoveType == EMoveType::HOMING_BOUNCE && CachedInstigator)
	{
		constexpr float InitSearchRadius = 2000.f;
		const FVector Origin = CachedInstigator->GetActorLocation();
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

			UAbilitySystemComponent* CandidateASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Candidate);
			if (!CandidateASC || !CandidateASC->HasMatchingGameplayTag(RSTags::Team_Enemy))
			{
				continue;
			}

			// 전방 반구 필터 (내적 > 0 = 시전자 앞쪽)
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

	SpawnProjectiles(ProjClass, InitData);

	if (CachedInstigator)
	{
		SpawnSkillFX(ExecData.LevelData.FXClass, CachedInstigator->GetActorLocation(), ExecData.LevelData.EffectRadius);
	}

	KHS_INFO(TEXT("ProjectileSpawn 단발 — SkillID: %s"), *ExecData.SkillID.ToString());
}

void UGA_CharacterSkill::SpawnSkillFX(TSoftObjectPtr<UNiagaraSystem> FXClass, FVector Location, float Radius,
	FGameplayTag ElementTag, float FXLifetime, FRotator Rotation)
{
	UNiagaraSystem* FX = FXClass.LoadSynchronous();
	if (!FX)
	{
		return;
	}

	UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(), FX, Location, Rotation, FVector(1.f), true, true);

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

	// FXLifetime: 0 이하면 DESTROY_FX_DELAY 사용 (버스트 FX 기본값)
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
	// GroundEffect는 즉발형 — SpawnPreview 플로우를 거치지 않아 PendingTargetLocation이 유효하지 않음.
	// PlayerController의 현재 마우스 에임 위치를 직접 사용.
	FVector SpawnLoc = FVector::ZeroVector;
	if (const ARSPlayerController* PC = Cast<ARSPlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		SpawnLoc = PC->GetCachedAimLocation();
	}
	else if (CachedInstigator)
	{
		SpawnLoc = CachedInstigator->GetActorLocation();
		KHS_WARN(TEXT("ARSPlayerController 캐스트 실패 — 시전자 위치 폴백. SkillID: %s"), *ExecData.SkillID.ToString());
	}

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

	// 장판 Actor가 독립 수명 관리 — GA는 즉시 종료 (몽타주 없는 경우)
	if (!CastingMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}
