// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/RSBaseWidget.h"
#include "GameplayEffectTypes.h"
#include "BossHPBarWidget.generated.h"

class UProgressBar;
class UAbilitySystemComponent;

/**
 * UBossHPBarWidget
 *
 * - 보스 스폰 시 UIManagerSubsystem이 PERSISTENT 레이어에 열기
 */
UCLASS()
class ROASTSTAFFGAS_API UBossHPBarWidget : public URSBaseWidget
{
	GENERATED_BODY()

public:
	UBossHPBarWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void NativeDestruct() override;

public:
	/** EnemySpawner가 보스 스폰 직후 호출 */
	void BindToASC(UAbilitySystemComponent* InASC, float InPhase2Ratio);

	FORCEINLINE bool IsClosing() const { return bIsClosing; }
	
private:
	// ── ASC 콜백 ────────────────────────────────────────────────────────────
	void OnCurrentHPChanged(const FOnAttributeChangeData& Data);
	void OnMaxHPChanged(const FOnAttributeChangeData& Data);

	// ── 내부 갱신 헬퍼 ──────────────────────────────────────────────────────
	void UpdateProgressBars();
	void UpdateGhostBar(float InDeltaTime);
	/** HP 비율이 Phase2Ratio 이하로 최초 진입 시 호출 */
	void OnPhase2Entered();
	/** HP=0 감지 시 FadeOut 시작 (Anim_FadeOut 없으면 즉시 정리) */
	void TriggerFadeOut();

	/** FadeOut 애니메이션 완료 콜백 — UMS CloseUIByID 호출 */
	UFUNCTION()
	void OnFadeOutFinished();

	float CalcPercent(float InHP) const;

private:
	// ── 바인딩 위젯 ─────────────────────────────────────────────────────────
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> PBar_BossHP;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> PBar_Ghost;
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> Anim_FadeOut;

	// ── 조정 가능 파라미터 (BP에서 설정) ────────────────────────────────────
	UPROPERTY(EditDefaultsOnly, Category = "MY|BossHPBar", meta = (ClampMin = "0.0"))
	float GhostDelayTime = 0.8f;
	UPROPERTY(EditDefaultsOnly, Category = "MY|BossHPBar", meta = (ClampMin = "0.1"))
	float InterpSpeed_Ghost = 3.0f;

	// ── 런타임 상태 ─────────────────────────────────────────────────────────
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> CachedASC;

	float CurrentHP     = 0.f;
	float CurrentMaxHP  = 0.f;
	float GhostHP       = 0.f;
	float GhostDelayTimer = 0.f;
	float Phase2Ratio   = 0.5f;

	bool bPhase2Triggered = false;
	/** FadeOut 애니메이션 진행 중 여부 — 이중 Close 방지 */
	bool bIsClosing = false;
};
