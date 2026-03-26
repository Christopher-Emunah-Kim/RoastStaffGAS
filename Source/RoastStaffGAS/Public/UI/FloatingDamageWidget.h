// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FloatingDamageWidget.generated.h"

class UTextBlock;

/**
 * UFloatingDamageWidget
 *
 * - 피격 시 데미지 수치를 뷰포트에 플로팅하는 위젯
 * - 에너미 피격(플레이어→에너미), 플레이어 피격(에너미→플레이어) 양방향 사용
 * - ARSPlayerController가 풀을 소유하며 3D→2D 프로젝션으로 스크린 좌표를 결정
 * - FadeAnimation 완료 시 자동으로 PlayerController 풀에 반납
 */
UCLASS()
class ROASTSTAFFGAS_API UFloatingDamageWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 데미지 수치를 텍스트에 설정하고 FadeAnimation을 재생 */
	void PlayFloatAnimation(float Damage);

private:
	UFUNCTION()
	void OnFadeAnimationFinished();

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_Amount;
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> FadeAnimation;
};