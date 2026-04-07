// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayEffectTypes.h"
#include "EnemyHPBarWidget.generated.h"

class UProgressBar;
class UImage;
class UAbilitySystemComponent;
class UTextBlock;

/**
 * UEnemyHPBarWidget
 *
 * - 에너미 WidgetComponent에 부착되는 월드 스페이스 체력바
 * - BindToASC() 호출로 ASC 델리게이트를 직접 구독
 * - GhostBar: 피격 후 딜레이를 두고 보간하여 추종
 */
UCLASS()
class ROASTSTAFFGAS_API UEnemyHPBarWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void NativeDestruct() override;

public:
	/** ASC에 바인딩하고 초기 HP 값으로 위젯을 렌더링 */
	void BindToASC(UAbilitySystemComponent* InASC);
	/** EnemyID를 Txt_EnemyName에 표시 (디버그 식별용) */
	void SetEnemyName(const FText& InName);
	
private:
	/** ASC 어트리뷰트 변경 델리게이트 콜백 */
	void OnCurrentHPChanged(const FOnAttributeChangeData& Data);
	void OnMaxHPChanged(const FOnAttributeChangeData& Data);

	/** GhostBar를 TargetHealth 방향으로 보간 */
	void UpdateGhostBar(float InDeltaTime);
	/** LowHealth 상태 진입/탈출 시 DangerGlow 및 애니메이션 갱신 */
	void CheckLowHealthState();
	/** Percent 계산 헬퍼 — MaxHP = 0 방어 */
	float CalcPercent(float InHealth) const;

private:
	// 바인딩 위젯 
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> PBar_Health;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> PBar_Ghost;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Img_DangerGlow;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Txt_EnemyName;
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> Anim_LowHealth;
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> Anim_HitShake;

	// 조정 가능 파라미터 (BP에서 설정) 
	UPROPERTY(EditDefaultsOnly, Category = "MY|HPBar", meta = (ClampMin = "0.0"))
	float GhostDelayTime = 0.8f;
	UPROPERTY(EditDefaultsOnly, Category = "MY|HPBar", meta = (ClampMin = "0.1"))
	float InterpSpeed_Ghost = 3.0f;
	UPROPERTY(EditDefaultsOnly, Category = "MY|HPBar", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float LowHealthThreshold = 0.3f;

	// 런타임 상태 
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> CachedASC;

	float TargetHealth     = 0.f;
	float GhostHealth      = 0.f;
	float CurrentMaxHealth = 0.f;
	float GhostDelayTimer  = 0.f;
	bool  bIsLowHealth     = false;
};
