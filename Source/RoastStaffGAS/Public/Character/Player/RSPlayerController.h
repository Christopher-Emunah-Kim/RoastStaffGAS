// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "RSPlayerController.generated.h"

/**
 * ARSPlayerController
 * - 플레이어 입력처리 및 UI 관리 책임
 */

class URSHUDWidget;
class UInputAction;
class UInputMappingContext;
struct FInputActionValue;


UCLASS()
class ROASTSTAFFGAS_API ARSPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void SetupInputComponent() override;
	virtual void PlayerTick(float DeltaTime) override;

private:
	//========================================================
	// UI 관리
	//========================================================
	UFUNCTION()
	void OnSlotUpdated(int32 SlotIndex);
	void RefreshSlotUI(int32 SlotIndex);
	
	//========================================================
	// 입력 처리
	//========================================================
	bool HandleMouseAim();
	void OnMove(const FInputActionValue& Value);
	void OnMouseAim(const FInputActionValue& Value);
	void OnShootStart(const FInputActionValue& Value);
	void OnSlotActivate(const FInputActionValue& Value, int32 SlotIndex);

protected:
	// UI
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<URSHUDWidget> HUDWidgetClass;
	UPROPERTY()
	TObjectPtr<URSHUDWidget> CachedHUDUI;
	
	// 입력 에셋
	UPROPERTY(EditDefaultsOnly, Category = "MY|Input")
	TObjectPtr<UInputMappingContext> IMC;
	UPROPERTY(EditDefaultsOnly, Category = "MY|Input")
	TObjectPtr<UInputAction> IA_Move;
	UPROPERTY(EditDefaultsOnly, Category = "MY|Input")
	TObjectPtr<UInputAction> IA_MouseAim;
	UPROPERTY(EditDefaultsOnly, Category = "MY|Input")
	TObjectPtr<UInputAction> IA_Attack;
	UPROPERTY(EditDefaultsOnly, Category = "MY|Input")
	TObjectPtr<UInputAction> IA_Slot1;
	UPROPERTY(EditDefaultsOnly, Category = "MY|Input")
	TObjectPtr<UInputAction> IA_Slot2;
	UPROPERTY(EditDefaultsOnly, Category = "MY|Input")
	TObjectPtr<UInputAction> IA_Slot3;

	// 런타임 상태
	FVector CachedAimLocation = FVector::ZeroVector; // 에임 좌표 캐시 — RequestManualFire 시 전달
	FVector2D LastMoveInput = FVector2D::ZeroVector; // 마지막 이동 입력 — 대시 방향 계산용
	float AimAngle = 0.f;						     // 에임 각도 (Yaw)		

};
