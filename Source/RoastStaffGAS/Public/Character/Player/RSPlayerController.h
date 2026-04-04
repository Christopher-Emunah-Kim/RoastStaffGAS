// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "RSPlayerController.generated.h"

/**
 * ARSPlayerController
 * - 플레이어 입력처리 및 UI 관리 책임
 */

class UFloatingDamageWidget;
class UInputAction;
class UInputMappingContext;
struct FInputActionValue;
struct FWeaponCardDisplayData;


UCLASS()
class ROASTSTAFFGAS_API ARSPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void SetupInputComponent() override;
	virtual void PlayerTick(float DeltaTime) override;

public:
	FORCEINLINE FVector GetCachedAimLocation() const { return CachedAimLocation; }
	
	// 플로팅 데미지 풀 관리
	/** 피격 위치(WorldPos)를 스크린 좌표로 변환해 FloatingDamageWidget을 표시 */
	void SpawnFloatingDamage(FVector WorldPos, float Damage);

	/** 애니메이션 완료 후 위젯을 풀에 반납 */
	void ReturnFloatingDamageToPool(UFloatingDamageWidget* Widget);

private:
	/** BeginPlay에서 PoolInitialSize만큼 위젯을 미리 생성 */
	void PrewarmFloatingDamagePool();

	//========================================================
	// UI 관리
	//========================================================
	UFUNCTION()
	void OnSlotUpdated(int32 SlotIndex);
	void RefreshSlotUI(int32 SlotIndex);

	/** LevelUpSubsystem.OnWeaponCandidatesReadyDel 핸들러 — 레벨업 UI 오픈 + 게임 일시정지 */
	UFUNCTION()
	void OnWeaponCandidatesReady(const TArray<FWeaponCardDisplayData>& WeaponCards);
	/** LevelUpWeaponSelectWidget.OnWeaponSelectCompletedDel 핸들러 — 게임 재개 */
	UFUNCTION()
	void OnWeaponSelectCompleted();
	/** EquipmentSubsystem.OnSlotFull 핸들러 — 교체 UI 오픈 */
	UFUNCTION()
	void OnWeaponSlotFull(FName PendingWeaponID);
	/** WeaponReplaceWidget.OnReplaceCompletedDel 핸들러 — 레벨업 완료 통보 */
	UFUNCTION()
	void OnWeaponReplaceCompleted();

	//========================================================
	// 입력 처리
	//========================================================
	bool HandleMouseAim();
	void OnMove(const FInputActionValue& Value);
	void OnShootStart(const FInputActionValue& Value);
	void OnSlotActivate(const FInputActionValue& Value, int32 SlotIndex);

protected:
	// UI — FloatingDamageWidget
	UPROPERTY(EditDefaultsOnly, Category = "MY|UI")
	TSubclassOf<UFloatingDamageWidget> FloatingDamageWidgetClass;
	/** 사용 가능한 비활성 위젯 풀 */
	UPROPERTY()
	TArray<TObjectPtr<UFloatingDamageWidget>> FloatingDamagePool;
	/** 미리 생성할 위젯 수 */
	UPROPERTY(EditDefaultsOnly, Category = "MY|UI", meta = (ClampMin = "0"))
	int32 PoolInitialSize = 10;
	/** 동시 생성 가능한 위젯 총 상한 (prewarm + 온디맨드 합산) */
	UPROPERTY(EditDefaultsOnly, Category = "MY|UI", meta = (ClampMin = "1"))
	int32 PoolMaxSize = 30;

	// 입력 에셋
	UPROPERTY(EditDefaultsOnly, Category = "MY|Input")
	TObjectPtr<UInputMappingContext> IMC;
	UPROPERTY(EditDefaultsOnly, Category = "MY|Input")
	TObjectPtr<UInputAction> IA_Move;
	UPROPERTY(EditDefaultsOnly, Category = "MY|Input")
	TObjectPtr<UInputAction> IA_Attack;
	UPROPERTY(EditDefaultsOnly, Category = "MY|Input")
	TObjectPtr<UInputAction> IA_Slot1;
	UPROPERTY(EditDefaultsOnly, Category = "MY|Input")
	TObjectPtr<UInputAction> IA_Slot2;
	UPROPERTY(EditDefaultsOnly, Category = "MY|Input")
	TObjectPtr<UInputAction> IA_Slot3;

	// 런타임 상태
	FVector   CachedAimLocation  = FVector::ZeroVector;
	FVector2D LastMoveInput      = FVector2D::ZeroVector;
	float     AimAngle           = 0.f;

private:
	/** 총 생성된 FloatingDamageWidget 수 — 풀 상한 판단에 사용 */
	int32 TotalCreatedCount = 0;
	
};