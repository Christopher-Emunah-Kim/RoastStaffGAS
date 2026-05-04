// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/Abilities/GA_CharacterSkill_Charge.h"
#include "RoastStaffGAS.h"
#include "GAS/Tags/RSGameplayTags.h"
#include "Subsystems/SkillManagerSubsystem.h"
#include "Subsystems/PoolingSubsystem.h"
#include "Subsystems/UIManagerSubsystem.h"
#include "Character/BaseCharacter.h"
#include "Character/Player/RSPlayerController.h"
#include "Data/RuntimeDataStructs.h"
#include "Data/EnumUITypes.h"
#include "Objects/Data/RSCharacterSkillData.h"
#include "UI/RSHUDWidget.h"
#include "Objects/Projectile/BaseProjectile.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "System/LoggingSystem.h"

// ============================================================================
// OnAbilityActivated
// ============================================================================

void UGA_CharacterSkill_Charge::OnAbilityActivated(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	// SkillMgr에서 AimPreview 확정 위치 수신 (ConfirmSkillPreview가 이미 저장한 값)
	GET_WORLD_SUBSYSTEM(USkillManagerSubsystem, SkillMgr)
	CachedTargetLocation = SkillMgr->GetPendingTargetLocation();

	if (CachedTargetLocation.IsZero())
	{
		KHS_WARN(TEXT("CachedTargetLocation이 ZeroVector — 프리뷰 확정 없이 GA 발동됨. 스킵."));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const URSCharacterSkillData* SkillData = Cast<URSCharacterSkillData>(GetCurrentSourceObject());
	if (!ensureMsgf(SkillData, TEXT("OnAbilityActivated: SourceObject가 URSCharacterSkillData 아님")))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const FCharacterSkillExecData& ExecData = SkillMgr->GetSlotExecData(SkillData->SlotIndex);

	// 몽타주 있으면: Loop 섹션 재생 → HitCheck 노티파이 → StartCharging
	// 몽타주 없으면: StartCharging 즉시
	StartSkillWithMontage(ExecData, Handle, ActorInfo, ActivationInfo,
		[this, Handle, ActorInfo, ActivationInfo]()
		{
			StartCharging(Handle, ActorInfo, ActivationInfo);
		},
		false  // 차징 중 이동 허용 — DisableMovement 적용 안 함
	);
}

// ============================================================================
// CancelAbility
// ============================================================================

void UGA_CharacterSkill_Charge::CancelAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateCancelAbility)
{
	CleanupCharging();
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
}

// ============================================================================
// StartCharging
// ============================================================================

void UGA_CharacterSkill_Charge::StartCharging(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	UAbilitySystemComponent* OwnerASC = GetOwnerASC();
	if (!ensureMsgf(OwnerASC, TEXT("StartCharging: OwnerASC null")))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// ExecData에서 MaxChargeTime 읽기
	const URSCharacterSkillData* SkillData = Cast<URSCharacterSkillData>(GetCurrentSourceObject());
	if (!ensureMsgf(SkillData, TEXT("StartCharging: SourceObject가 URSCharacterSkillData 아님")))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	GET_WORLD_SUBSYSTEM(USkillManagerSubsystem, SkillMgr)
	const FCharacterSkillExecData& ExecData = SkillMgr->GetSlotExecData(SkillData->SlotIndex);

	const float MaxChargeTime = FMath::Max(0.1f, ExecData.MaxChargeTime);

	// 차징 상태 태그 부여
	OwnerASC->AddLooseGameplayTag(RSTags::Skill_State_Charging);
	ChargeStartTime = GetWorld()->GetTimeSeconds();

	// 게이지 UI 표시
	ShowChargeGauge(MaxChargeTime);

	// Tag_ChargeRelease 이벤트 대기 (LMB 해제 시 PC가 송신)
	ChargeEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, RSTags::Skill_Event_ChargeRelease);
	ChargeEventTask->EventReceived.AddDynamic(this, &UGA_CharacterSkill_Charge::OnChargeEventReceived);
	ChargeEventTask->ReadyForActivation();

	// 타임아웃 — MaxChargeTime 만료 시 자동 발사
	TWeakObjectPtr<UGA_CharacterSkill_Charge> WeakThis(this);
	GetWorld()->GetTimerManager().SetTimer(
		ChargeTimeoutHandle,
		[WeakThis]()
		{
			if (!WeakThis.IsValid())
			{
				return;
			}
			KHS_INFO(TEXT("스나이프 차징 타임아웃 — 자동 발사"));
			WeakThis->FireSnipeProjectile(
				WeakThis->GetCurrentAbilitySpecHandle(),
				WeakThis->GetCurrentActorInfo(),
				WeakThis->GetCurrentActivationInfo());
		},
		MaxChargeTime,
		false
	);

	KHS_INFO(TEXT("스나이프 차징 시작 — MaxChargeTime: %.1fs | 목표위치: %s"),
		MaxChargeTime, *CachedTargetLocation.ToString());
}

// ============================================================================
// OnChargeEventReceived
// ============================================================================

void UGA_CharacterSkill_Charge::OnChargeEventReceived(FGameplayEventData Payload)
{
	// FireSnipeProjectile 내부의 ChargeStartTime sentinel이 중복 발사를 차단함
	FireSnipeProjectile(
		GetCurrentAbilitySpecHandle(),
		GetCurrentActorInfo(),
		GetCurrentActivationInfo());
}

// ============================================================================
// FireSnipeProjectile
// ============================================================================

void UGA_CharacterSkill_Charge::FireSnipeProjectile(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	// 중복 발사 방지 — 타이머 이미 클리어됐으면 이미 처리된 것
	if (ChargeStartTime <= 0.f)
	{
		return;
	}

	const URSCharacterSkillData* SkillData = Cast<URSCharacterSkillData>(GetCurrentSourceObject());
	if (!SkillData)
	{
		CleanupCharging();
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	GET_WORLD_SUBSYSTEM(USkillManagerSubsystem, SkillMgr)
	const FCharacterSkillExecData& ExecData = SkillMgr->GetSlotExecData(SkillData->SlotIndex);

	// ── 차징 배율 계산 ─────────────────────────────────────────────────────
	const float MaxChargeTime  = FMath::Max(0.1f, ExecData.MaxChargeTime);
	const float Elapsed        = GetWorld()->GetTimeSeconds() - ChargeStartTime;
	const float ElapsedRatio   = FMath::Clamp(Elapsed / MaxChargeTime, 0.f, 1.f);

	// DamageMultiplierMin(DT) ~ DamageMultiplierMax(constexpr) 선형 보간
	float DmgMultiplier = FMath::Lerp(ExecData.DamageMultiplier, DamageMultiplierMax, ElapsedRatio);

	// 퍼펙트 존(>=80%) 보너스 — 판정은 GA 전담
	if (ElapsedRatio >= PerfectZoneThreshold)
	{
		DmgMultiplier *= ExecData.PerfectZoneBonus;
		KHS_INFO(TEXT("스나이프 퍼펙트 존! Ratio=%.2f | DmgMultiplier=%.2f"), ElapsedRatio, DmgMultiplier);
	}

	// 발사 몽타주 섹션 전환 — Loop → Shoot
	if (GetCurrentMontage())
	{
		MontageJumpToSection(FName("Shoot"));
	}

	// ── 투사체 발사 ────────────────────────────────────────────────────────
	TSubclassOf<ABaseProjectile> LoadedClass;
	if (!LoadRequiredClass(ExecData.ProjectileClass, LoadedClass, ExecData.SkillID))
	{
		CleanupCharging();
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (CachedInstigator)
	{
		const FVector PlayerLoc = CachedInstigator->GetActorLocation();
		// CachedTargetLocation 방향으로 발사 (높이 차이 무시 — 2D 방향 기준)
		FVector FireDir = (CachedTargetLocation - PlayerLoc).GetSafeNormal2D();
		if (FireDir.IsNearlyZero())
		{
			FireDir = CachedInstigator->GetActorForwardVector();
		}

		FProjectileInitData InitData  = BuildProjectileInitData(ExecData, LoadedClass);
		// 차징 배율로 데미지 재계산 (기본 GetSkillDamageAmount는 DamageMultiplier 고정 사용)
		const float BaseATK = GetSkillDamageAmount(ExecData) / FMath::Max(0.001f, ExecData.DamageMultiplier);
		InitData.Amount     = BaseATK * DmgMultiplier;

		const FVector SpawnLoc    = PlayerLoc + FireDir * SPAWN_OFFSET;
		const FRotator SpawnRot   = FireDir.ToOrientationRotator();

		GET_WORLD_SUBSYSTEM(UPoolingSubsystem, PoolSys)
		ABaseProjectile* Projectile = PoolSys->SpawnPooledActor<ABaseProjectile>(
			LoadedClass, FTransform(SpawnRot, SpawnLoc));

		if (Projectile)
		{
			Projectile->SetOwner(const_cast<ABaseCharacter*>(CachedInstigator.Get()));
			Projectile->SetInstigator(const_cast<ABaseCharacter*>(CachedInstigator.Get()));
			Projectile->InitProjectile(InitData);

			KHS_INFO(TEXT("스나이프 발사 — Ratio=%.2f | DmgMultiplier=%.2f | 위치: %s"),
				ElapsedRatio, DmgMultiplier, *SpawnLoc.ToString());
		}
		else
		{
			KHS_WARN(TEXT("스나이프 투사체 스폰 실패"));
		}

		SpawnSkillFX(ExecData.SkillFX, PlayerLoc, ExecData.EffectRadius, ExecData.ElementTag);
	}

	CleanupCharging();

	// 몽타주 있으면: Shoot 섹션 완료 후 OnCastingMontageEnded → EndAbility
	// 몽타주 없으면: 직접 종료
	if (!GetCurrentMontage())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

// ============================================================================
// CleanupCharging
// ============================================================================

void UGA_CharacterSkill_Charge::CleanupCharging()
{
	// 중복 클리어 방지 — ChargeStartTime을 sentinel로 사용
	if (ChargeStartTime <= 0.f)
	{
		return;
	}
	ChargeStartTime = 0.f;

	GetWorld()->GetTimerManager().ClearTimer(ChargeTimeoutHandle);

	if (ChargeEventTask)
	{
		ChargeEventTask->EndTask();
		ChargeEventTask = nullptr;
	}

	UAbilitySystemComponent* OwnerASC = GetOwnerASC();
	if (OwnerASC && OwnerASC->HasMatchingGameplayTag(RSTags::Skill_State_Charging))
	{
		OwnerASC->RemoveLooseGameplayTag(RSTags::Skill_State_Charging);
	}

	HideChargeGauge();
}

// ============================================================================
// HUD 헬퍼
// ============================================================================

void UGA_CharacterSkill_Charge::ShowChargeGauge(float MaxChargeTime) const
{
	GET_GI_SUBSYSTEM_FROM(UUIManagerSubsystem, UMS, GetWorld()->GetGameInstance())
	URSHUDWidget* HUD = Cast<URSHUDWidget>(UMS->GetWidgetByID(EUIID::HUD));
	if (!HUD)
	{
		KHS_WARN(TEXT("ShowChargeGauge: HUD Widget 없음"));
		return;
	}
	HUD->ShowChargeGauge(MaxChargeTime);
}

void UGA_CharacterSkill_Charge::HideChargeGauge() const
{
	GET_GI_SUBSYSTEM_FROM(UUIManagerSubsystem, UMS, GetWorld()->GetGameInstance())
	URSHUDWidget* HUD = Cast<URSHUDWidget>(UMS->GetWidgetByID(EUIID::HUD));
	if (!HUD)
	{
		return;
	}
	HUD->HideChargeGauge();
}
