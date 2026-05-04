// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/GA_CharacterSkill.h"
#include "GA_CharacterSkill_Charge.generated.h"

class UAbilityTask_WaitGameplayEvent;

/**
 * UGA_CharacterSkill_Charge
 * ChargeAndRelease 전용 GA 서브클래스 — 스나이프 스킬.
 *
 * 흐름: AimPreview 확정(LMB) → GA 발동 → 차징 시작
 *       LMB 해제 → Tag_ChargeRelease 이벤트 수신 → 배율 계산 → Pierce 발사
 *
 * 퍼펙트 존(80%~100%) 판정은 이 클래스 전담 — Widget은 표시 전용.
 */
UCLASS()
class ROASTSTAFFGAS_API UGA_CharacterSkill_Charge : public UGA_CharacterSkill
{
	GENERATED_BODY()

protected:
	virtual void OnAbilityActivated(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) override;

	virtual void CancelAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateCancelAbility) override;

private:
	// ── 차징 이벤트 콜백 ───────────────────────────────────────────────────────
	/** WaitGameplayEvent(Tag_ChargeRelease) 수신 시 호출 */
	UFUNCTION()
	void OnChargeEventReceived(FGameplayEventData Payload);

	// ── 내부 헬퍼 ─────────────────────────────────────────────────────────────
	/** 차징 시작: 태그 부여 + 게이지 표시 + 타임아웃 타이머 + 이벤트 대기 */
	void StartCharging(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo);

	/** 발사 처리: ElapsedRatio → DamageMultiplier 계산 → Pierce 투사체 스폰 */
	void FireSnipeProjectile(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo);

	/** 차징 정리: 게이지 숨김 + State.Charging 태그 제거 + 타이머 클리어 */
	void CleanupCharging();

	/** HUD에서 URSHUDWidget을 가져와 ShowChargeGauge 호출 */
	void ShowChargeGauge(float MaxChargeTime) const;
	/** HUD에서 URSHUDWidget을 가져와 HideChargeGauge 호출 */
	void HideChargeGauge() const;

private:
	// ── 차징 런타임 상태 ──────────────────────────────────────────────────────
	/** AimPreview 확정 위치 — SkillMgr::GetPendingTargetLocation()에서 수신 */
	FVector CachedTargetLocation = FVector::ZeroVector;
	/** 차징 시작 시각 (GetWorld()->GetTimeSeconds()) */
	float ChargeStartTime = 0.f;

	/** 타임아웃 시 자동 발사 타이머 */
	FTimerHandle ChargeTimeoutHandle;

	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> ChargeEventTask;

	// ── 상수 ──────────────────────────────────────────────────────────────────
	/** 최대 차징 시 데미지 배율 상한 */
	static constexpr float DamageMultiplierMax = 2.0f;
	/** 퍼펙트 존 진입 임계 비율 (80%) */
	static constexpr float PerfectZoneThreshold = 0.8f;
};
