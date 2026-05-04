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
struct FLevelUpCardDisplayData;


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
	// 플로팅 데미지 풀 관리
	/** 피격 위치(WorldPos)를 스크린 좌표로 변환해 FloatingDamageWidget을 표시 */
	void SpawnFloatingDamage(FVector WorldPos, float Damage);
	/** 애니메이션 완료 후 위젯을 풀에 반납 */
	void ReturnFloatingDamageToPool(UFloatingDamageWidget* Widget);
	/** 플레이어 피격 시 HUD 데미지 인디케이터 재생 */
	void FlashHUDDamageIndicator();
	
	FORCEINLINE TSubclassOf<UFloatingDamageWidget> GetFloatingDamageWidgetClass() const { return FloatingDamageWidgetClass; }
	FORCEINLINE FVector GetCachedAimLocation() const { return CachedAimLocation; }
private:
	void HandleInputContext();
	void BindSubsystemDelegates();
	
	//========================================================
	// UI 관리
	//========================================================
	void OpenHUDUI();
	void InitSlotUIGuarantee();
	
	UFUNCTION()
	void OnSlotUpdated(int32 SlotIndex);
	void RefreshSlotUI(int32 SlotIndex);

	UFUNCTION()
	void OnSkillSlotUpdated(int32 SlotIndex);
	void RefreshSkillSlotUI(int32 SlotIndex);

	/** LevelUpSubsystem.OnCardPoolReadyDel 핸들러 — 레벨업 UI 오픈 + 게임 일시정지 */
	UFUNCTION()
	void OnCardPoolReady(const TArray<FLevelUpCardDisplayData>& Cards);
	/** LevelUpWeaponSelectWidget.OnWeaponSelectCompletedDel 핸들러 — 게임 재개 */
	UFUNCTION()
	void OnWeaponSelectCompleted();
	/** EquipmentSubsystem.OnSlotFull 핸들러 — 교체 UI 오픈 */
	UFUNCTION()
	void OnWeaponSlotFull(FName PendingWeaponID);
	/** WeaponReplaceWidget.OnReplaceCompletedDel 핸들러 — 레벨업 완료 통보 */
	UFUNCTION()
	void OnWeaponReplaceCompleted();
	/** PassiveSlotSubsystem.OnPassiveSlotChangedDel 핸들러 — HUD 패시브 UI 갱신 트리거 */
	UFUNCTION()
	void OnPassiveSlotChanged();

	//========================================================
	// 입력 처리
	//========================================================
	bool HandleMouseAim();
	void OnMove(const FInputActionValue& Value);
	/** LMB — 스킬 프리뷰 활성 시 SkillManagerSubsystem::ConfirmSkillPreview, 비활성 시 무입력 */
	void OnConfirm(const FInputActionValue& Value);
	/** 숫자키 1~6 — SkillManagerSubsystem::ActivateSkillSlot(SlotIndex 0~5) */
	void OnSkill1(const FInputActionValue& Value);
	void OnSkill2(const FInputActionValue& Value);
	void OnSkill3(const FInputActionValue& Value);
	void OnSkill4(const FInputActionValue& Value);
	void OnSkill5(const FInputActionValue& Value);
	void OnSkill6(const FInputActionValue& Value);
	/** RMB — 스킬 프리뷰 취소 */
	void OnSkillCancel(const FInputActionValue& Value);
	/** LMB Released — State.Charging 태그 보유 시 ChargeRelease 이벤트 송신 */
	void OnChargeInputReleased(const FInputActionValue& Value);
	/** Tab — 캐릭터 스탯 팝업 토글 */
	void OnStatPopupToggle(const FInputActionValue& Value);

protected:
	// UI — FloatingDamageWidget
	UPROPERTY(EditDefaultsOnly, Category = "MY|UI")
	TSubclassOf<UFloatingDamageWidget> FloatingDamageWidgetClass;

private:
	// 입력 에셋
	UPROPERTY(EditDefaultsOnly, Category = "MY|Input")
	TObjectPtr<UInputMappingContext> IMC;
	UPROPERTY(EditDefaultsOnly, Category = "MY|Input")
	TObjectPtr<UInputAction> IA_Move;
	UPROPERTY(EditDefaultsOnly, Category = "MY|Input")
	TObjectPtr<UInputAction> IA_Attack;
	UPROPERTY(EditDefaultsOnly, Category = "MY|Input")
	TObjectPtr<UInputAction> IA_Skill1;
	UPROPERTY(EditDefaultsOnly, Category = "MY|Input")
	TObjectPtr<UInputAction> IA_Skill2;
	UPROPERTY(EditDefaultsOnly, Category = "MY|Input")
	TObjectPtr<UInputAction> IA_Skill3;
	UPROPERTY(EditDefaultsOnly, Category = "MY|Input")
	TObjectPtr<UInputAction> IA_Skill4;
	UPROPERTY(EditDefaultsOnly, Category = "MY|Input")
	TObjectPtr<UInputAction> IA_Skill5;
	UPROPERTY(EditDefaultsOnly, Category = "MY|Input")
	TObjectPtr<UInputAction> IA_Skill6;
	UPROPERTY(EditDefaultsOnly, Category = "MY|Input")
	TObjectPtr<UInputAction> IA_SkillCancel;
	UPROPERTY(EditDefaultsOnly, Category = "MY|Input")
	TObjectPtr<UInputAction> IA_StatPopup;

	// 런타임 상태
	FVector   CachedAimLocation  = FVector::ZeroVector;
	FVector2D LastMoveInput      = FVector2D::ZeroVector;
	float     AimAngle           = 0.f;

private:
	
};