// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/Calculations/RS_DamageExecCalc.h"
#include "GAS/Attributes/BaseAttributeSet.h"
#include "GAS/Attributes/PlayerAttributeSet.h"
#include "GAS/Tags/RSGameplayTags.h"
#include "AbilitySystemComponent.h"

// ============================================================================
// 어트리뷰트 캡처 정의 (정적 초기화 — 생성자에서 등록)
// ============================================================================
struct FDamageExecCaptures
{
	// Source 어트리뷰트 — 플레이어→에너미 공식용
	DECLARE_ATTRIBUTE_CAPTUREDEF(ATK)
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalRate)
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalDamage)

	// Target 어트리뷰트 — 에너미→플레이어 공식용
	DECLARE_ATTRIBUTE_CAPTUREDEF(DEF)

	FDamageExecCaptures()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(UPlayerAttributeSet, ATK, Source, false)
		DEFINE_ATTRIBUTE_CAPTUREDEF(UPlayerAttributeSet, CriticalRate, Source, false)
		DEFINE_ATTRIBUTE_CAPTUREDEF(UPlayerAttributeSet, CriticalDamage, Source, false)
		DEFINE_ATTRIBUTE_CAPTUREDEF(UPlayerAttributeSet, DEF, Target, false)
	}
};

static const FDamageExecCaptures& GetCaptures()
{
	static FDamageExecCaptures Captures;
	return Captures;
}

// ============================================================================
// 생성자
// ============================================================================
URS_DamageExecCalc::URS_DamageExecCalc()
{
	const FDamageExecCaptures& Captures = GetCaptures();
	RelevantAttributesToCapture.Add(Captures.ATKDef);
	RelevantAttributesToCapture.Add(Captures.CriticalRateDef);
	RelevantAttributesToCapture.Add(Captures.CriticalDamageDef);
	RelevantAttributesToCapture.Add(Captures.DEFDef);
}

// ============================================================================
// Execute — Source 팀 태그로 공식 분기
// ============================================================================
void URS_DamageExecCalc::Execute_Implementation(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvalParams;
	EvalParams.SourceTags = SourceTags;
	EvalParams.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	const bool bIsPlayerSource = SourceTags->HasTagExact(RSTags::Team_Player);

	if (bIsPlayerSource)
	{
		// ─────────────────────────────────────────────────────────
		// 플레이어→에너미: BaseDmg × (1 + ATK/100) × CritMult
		// ─────────────────────────────────────────────────────────
		const float BaseDmg = Spec.GetSetByCallerMagnitude(RSTags::Data_WeaponBaseDamage, false, 0.f);
		if (BaseDmg <= 0.f)
		{
			return;
		}

		float ATK = 0.f;
		float CritRate = 0.f;
		float CritDmg  = 1.5f;

		const FDamageExecCaptures& Captures = GetCaptures();
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Captures.ATKDef, EvalParams, ATK);
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Captures.CriticalRateDef, EvalParams, CritRate);
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Captures.CriticalDamageDef, EvalParams, CritDmg);

		ATK      = FMath::Max(0.f, ATK);
		CritRate = FMath::Clamp(CritRate, 0.f, 1.f);
		CritDmg  = FMath::Max(1.f, CritDmg);

		const bool bCrit = FMath::FRand() < CritRate;
		const float CritMult  = bCrit ? CritDmg : 1.f;
		const float FinalDamage = BaseDmg * (1.f + ATK / 100.f) * CritMult;

		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
			UBaseAttributeSet::GetCurrentHPAttribute(),
			EGameplayModOp::Additive,
			-FinalDamage
		));
	}
	else
	{
		// ─────────────────────────────────────────────────────────
		// 에너미→플레이어: max(1, EnemyDmg - DEF)
		// ─────────────────────────────────────────────────────────
		const float EnemyDmg = Spec.GetSetByCallerMagnitude(RSTags::Data_EnemyAttackDamage, false, 0.f);
		if (EnemyDmg <= 0.f)
		{
			return;
		}

		float DEF = 0.f;
		const FDamageExecCaptures& Captures = GetCaptures();
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Captures.DEFDef, EvalParams, DEF);
		DEF = FMath::Max(0.f, DEF);

		const float FinalDamage = FMath::Max(1.f, EnemyDmg - DEF);

		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
			UBaseAttributeSet::GetCurrentHPAttribute(),
			EGameplayModOp::Additive,
			-FinalDamage
		));
	}
}
