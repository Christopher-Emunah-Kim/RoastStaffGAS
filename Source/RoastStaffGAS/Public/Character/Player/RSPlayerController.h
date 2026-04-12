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

protected:
	// UI — FloatingDamageWidget
	UPROPERTY(EditDefaultsOnly, Category = "MY|UI")
	TSubclassOf<UFloatingDamageWidget> FloatingDamageWidgetClass;

	// 입력 에셋
	UPROPERTY(EditDefaultsOnly, Category = "MY|Input")
	TObjectPtr<UInputMappingContext> IMC;
	UPROPERTY(EditDefaultsOnly, Category = "MY|Input")
	TObjectPtr<UInputAction> IA_Move;
	/** M-5 진입점 — 캐릭터 스킬 슬롯 1 (Q키) */
	UPROPERTY(EditDefaultsOnly, Category = "MY|Input")
	TObjectPtr<UInputAction> IA_SkillQ;
	/** M-5 진입점 — 캐릭터 스킬 슬롯 2 (E키) */
	UPROPERTY(EditDefaultsOnly, Category = "MY|Input")
	TObjectPtr<UInputAction> IA_SkillE;

	// 런타임 상태
	FVector   CachedAimLocation  = FVector::ZeroVector;
	FVector2D LastMoveInput      = FVector2D::ZeroVector;
	float     AimAngle           = 0.f;

private:
};