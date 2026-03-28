// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayEffectTypes.h"
#include "PlayerStatusBarWidget.generated.h"

class UProgressBar;
class UTextBlock;
class UImage;
class UAbilitySystemComponent;
class ARSPlayerState;

/**
 * UPlayerStatusBarWidget
 *
 * - WBP_HUD의 자식 위젯으로 삽입되는 플레이어 체력바
 * - NativeConstruct에서 NextTick으로 Player ASC에 자체 바인딩
 * - GhostBar: 피격 후 딜레이를 두고 보간하여 추종
 * - LowHealth: HP 비율이 LowHealthThreshold 이하일 때 애니메이션 활성화
 * - HitShake: HP 감소 감지 시 Anim_HitShake 재생
 */
UCLASS()
class ROASTSTAFFGAS_API UPlayerStatusBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** ASC에 바인딩하고 초기 HP 값으로 위젯을 렌더링 */
	void BindToASC(UAbilitySystemComponent* InASC);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void NativeDestruct() override;

private:
	/** NativeConstruct NextTick에서 호출 — Player ASC 경로 탐색 후 BindToASC */
	void BindToPlayerASC();
	void InitializeEXPBar();
	void BindToAttributeChangeDelegates();

	/** ASC 어트리뷰트 변경 델리게이트 콜백 */
	void OnCurrentHPChanged(const FOnAttributeChangeData& Data);
	void OnMaxHPChanged(const FOnAttributeChangeData& Data);
	// EXP/Level 어트리뷰트 변경 콜백
	void OnEXPAttrChanged(const FOnAttributeChangeData& Data);
	void OnLevelAttrChanged(const FOnAttributeChangeData& Data);

	/** GhostBar를 TargetHealth 방향으로 보간 */
	void UpdateGhostBar(float InDeltaTime);
	/** EXPBar를 레벨업 후 시간을 두고 보간*/
	void UpdateExpBar(float InDeltaTime);
	/** LowHealth 상태 진입/탈출 시 애니메이션 갱신 */
	void CheckLowHealthState();
	/** HP 감소 감지 시 HitShake 애니메이션 재생 */
	void TriggerHitShake();
	/** Percent 계산 헬퍼 — MaxHP = 0 방어 */
	float CalcPercent(float InHealth) const;

protected:
	// 바인딩 위젯
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_PlayerName; // 캐릭터 이름 표시. BP에서 지정 (C++ 업데이트 DEFERRED)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> PBar_Health;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> PBar_Ghost;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> PBar_Exp;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Img_DangerGlow;
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> Anim_LowHealth; // 체력 낮을 때 재생되는 애니메이션
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> Anim_HitShake; // 피격 시 체력바 흔들리는 애니메이션

private:
	// 조정 가능 파라미터 (BP에서 설정)
	UPROPERTY(EditDefaultsOnly, Category = "MY|HPBar", meta = (ClampMin = "0.0"))
	float GhostDelayTime = 0.8f;
	UPROPERTY(EditDefaultsOnly, Category = "MY|HPBar", meta = (ClampMin = "0.1"))
	float InterpSpeed_Ghost = 3.0f;
	UPROPERTY(EditDefaultsOnly, Category = "MY|HPBar", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float LowHealthThreshold = 0.3f;
	UPROPERTY(EditDefaultsOnly, Category = "MY|EXPBar", meta = (ClampMin = "0.1"))
	float EXPLerpSpeed = 4.0f;

	// 런타임 상태
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> CachedASC;

	float TargetHealth     = 0.f;
	float GhostHealth      = 0.f;
	float CurrentMaxHealth = 0.f;
	float GhostDelayTimer  = 0.f;
	bool  bIsLowHealth     = false;

	float TargetEXPPercent  = 0.f;
	float CurrentEXPPercent = 0.f;
	float LerpStartPercent  = 0.f;
	bool  bIsLerpingEXP     = false;
};
