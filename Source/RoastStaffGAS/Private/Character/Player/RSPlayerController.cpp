// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/RSPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

#include "RoastStaffGAS.h"
#include "Subsystems/EquipmentSubsystem.h"
#include "Subsystems/LevelUpSubsystem.h"
#include "Subsystems/PoolingSubsystem.h"
#include "Subsystems/UIManagerSubsystem.h"
#include "UI/RSHUDWidget.h"
#include "UI/WeaponSlotContainerWidget.h"
#include "UI/WeaponSlotWidget.h"
#include "UI/FloatingDamageWidget.h"
#include "UI/LevelUpWeaponSelectWidget.h"
#include "UI/WeaponReplaceWidget.h"
#include "Data/EnumUITypes.h"
#include "Data/RuntimeDataStructs.h"

void ARSPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	//IMC 등록
	UEnhancedInputLocalPlayerSubsystem* Subsys =  ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (!Subsys || !IMC)
	{
		KHS_WARN(TEXT("EnhancedInput 시스템 로딩 실패 / IMC 에섯 미할당"));
		return;
	}
	Subsys->AddMappingContext(IMC, 0);
	SetShowMouseCursor(true);

	FInputModeGameOnly GameOnlyMode;                                                                 
	GameOnlyMode.SetConsumeCaptureMouseDown(false);                                                
	SetInputMode(GameOnlyMode);           
	
	//슬롯 델리게이트 구독
	GET_GI_SUBSYSTEM_FROM(UUIManagerSubsystem, UMS, GetGameInstance());
	GET_GI_SUBSYSTEM_FROM(UEquipmentSubsystem, EquipSys, GetGameInstance());
	GET_GI_SUBSYSTEM_FROM(ULevelUpSubsystem, LevelUpSys, GetGameInstance());

	EquipSys->OnSlotUpdatedDel.AddUniqueDynamic(this, &ARSPlayerController::OnSlotUpdated);
	EquipSys->OnSlotFull.AddUniqueDynamic(this, &ARSPlayerController::OnWeaponSlotFull);
	LevelUpSys->OnWeaponCandidatesReadyDel.AddUniqueDynamic(this, &ARSPlayerController::OnWeaponCandidatesReady);
	KHS_INFO(TEXT("델리게이트 구독 완료 (SlotUpdated / SlotFull / WeaponCandidatesReady)"));

	// UIManagerSettings 매핑 기반으로 HUD 오픈 — TSubclassOf 직접 참조 제거
	URSHUDWidget* HUDWidget = Cast<URSHUDWidget>(UMS->OpenUIByID(EUIID::HUD));
	if (!ensureMsgf(HUDWidget, TEXT("HUD Widget 오픈 실패 — UIManagerSettings HUD 매핑 확인 필요")))
	{
		return;
	}

}

void ARSPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GET_GI_SUBSYSTEM_FROM(UEquipmentSubsystem, EquipSys, GetGameInstance());
	EquipSys->OnSlotUpdatedDel.RemoveDynamic(this, &ARSPlayerController::OnSlotUpdated);

	GET_GI_SUBSYSTEM_FROM(ULevelUpSubsystem, LevelUpSys, GetGameInstance());
	EquipSys->OnSlotFull.RemoveDynamic(this, &ARSPlayerController::OnWeaponSlotFull);
	LevelUpSys->OnWeaponCandidatesReadyDel.RemoveDynamic(this, &ARSPlayerController::OnWeaponCandidatesReady);

	Super::EndPlay(EndPlayReason);
}

void ARSPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	UEnhancedInputComponent* EIC = CastChecked<UEnhancedInputComponent>(InputComponent);

	EIC->BindAction(IA_Move,  ETriggerEvent::Triggered, this, &ARSPlayerController::OnMove);
	EIC->BindAction(IA_Attack, ETriggerEvent::Started,   this, &ARSPlayerController::OnShootStart);
	EIC->BindAction(IA_Slot1, ETriggerEvent::Started, this, &ARSPlayerController::OnSlotActivate, 0);
	EIC->BindAction(IA_Slot2, ETriggerEvent::Started, this, &ARSPlayerController::OnSlotActivate, 1);
	EIC->BindAction(IA_Slot3, ETriggerEvent::Started, this, &ARSPlayerController::OnSlotActivate, 2);
}

void ARSPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	// 쿼터뷰 — 마우스 커서 방향으로 캐릭터 회전 / 마우스 좌표 캐싱
	if (!HandleMouseAim())
	{
		return;
	}
}

void ARSPlayerController::OnSlotUpdated(int32 SlotIndex)
{
	RefreshSlotUI(SlotIndex);
}

void ARSPlayerController::RefreshSlotUI(int32 SlotIndex)
{
	GET_GI_SUBSYSTEM_FROM(UEquipmentSubsystem, EquipSys, GetGameInstance());
	GET_GI_SUBSYSTEM_FROM(UUIManagerSubsystem, UMS, GetGameInstance());

	// BeginPlay에서 이미 열렸으므로 IsOpen() 체크 후 캐시 반환 — 재생성 없음
	URSHUDWidget* HUD = Cast<URSHUDWidget>(UMS->OpenUIByID(EUIID::HUD));
	if (!HUD)
	{
		KHS_WARN(TEXT("HUD Widget 획득 실패"));
		return;
	}

	UWeaponSlotContainerWidget* SlotContainer = HUD->GetSlotContainerWidget();
	if (!SlotContainer)
	{
		KHS_WARN(TEXT("WeaponSlotContainer 획득 실패"));
		return;
	}

	UWeaponSlotWidget* Slot = SlotContainer->GetSlotWidget(SlotIndex);
	if (!Slot)
	{
		KHS_WARN(TEXT("SlotWidget 획득 실패: %d"), SlotIndex);
		return;
	}

	const FWeaponSlotInstanceData* SlotData = EquipSys->GetSlotData(SlotIndex);
	Slot->UpdateSlot(SlotData);

	KHS_INFO(TEXT("Slot %d UI 갱신 — WeaponID: %s"), SlotIndex, SlotData ? *SlotData->SlotEquipData.WeaponID.ToString() : TEXT("null"));
}

bool ARSPlayerController::HandleMouseAim()
{
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)
	{
		KHS_WARN(TEXT("Failed to get Controlled Pawn"));
		return false;
	}
	
	FHitResult HitResult;
	GetHitResultUnderCursor(ECC_Visibility, false, HitResult);

	if (HitResult.bBlockingHit)
	{
		CachedAimLocation = HitResult.Location;
		const FRotator LookAt =	(HitResult.Location - ControlledPawn->GetActorLocation()).Rotation();
		AimAngle = LookAt.Yaw;
		ControlledPawn->SetActorRotation(FRotator(0.f, AimAngle, 0.f));
	}
	
	return true;
}

void ARSPlayerController::OnMove(const FInputActionValue& Value)
{
	const FVector2D Input = Value.Get<FVector2D>();
	LastMoveInput = Input;

	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)
	{
		KHS_WARN(TEXT("Failed to get Controlled Pawn"));
		return;
	}

	ControlledPawn->AddMovementInput(FVector::ForwardVector, Input.X);
	ControlledPawn->AddMovementInput(FVector::RightVector,   Input.Y);

}

void ARSPlayerController::OnShootStart(const FInputActionValue& Value)
{
	GET_GI_SUBSYSTEM_FROM(UEquipmentSubsystem, EquipSys, GetGameInstance());
	EquipSys->RequestManualFire(CachedAimLocation);
}

void ARSPlayerController::OnSlotActivate(const FInputActionValue& Value, int32 SlotIndex)
{
	GET_GI_SUBSYSTEM_FROM(UEquipmentSubsystem, EquipSys, GetGameInstance());
	EquipSys->RequestSlotActivate(SlotIndex);
}

// ============================================================================
// 레벨업 UI 관리
// ============================================================================

void ARSPlayerController::OnWeaponCandidatesReady(const TArray<FWeaponCardDisplayData>& WeaponCards)
{
	GET_GI_SUBSYSTEM_FROM(UUIManagerSubsystem, UMS, GetGameInstance());

	// UIManagerSettings LEVEL_UP 매핑으로 위젯 오픈 — 클래스 프로퍼티 직접 참조 제거
	ULevelUpWeaponSelectWidget* Widget = Cast<ULevelUpWeaponSelectWidget>(UMS->OpenUIByID(EUIID::LEVEL_UP));
	if (!ensureMsgf(Widget, TEXT("LevelUpWeaponSelectWidget 오픈 실패 — UIManagerSettings LEVEL_UP 매핑 확인 필요")))
	{
		GET_GI_SUBSYSTEM_FROM(ULevelUpSubsystem, LevelUpSys, GetGameInstance());
		LevelUpSys->NotifyWeaponSelectCompleted();
		return;
	}

	Widget->OnWeaponSelectCompletedDel.AddDynamic(this, &ARSPlayerController::OnWeaponSelectCompleted);
	Widget->SetCandidates(WeaponCards);

	// 게임 일시정지 (추후 StageSystem 위임으로 교체 예정)
	if (UWorld* World = GetWorld())
	{
		World->GetWorldSettings()->TimeDilation = 0.f;
	}

	KHS_INFO(TEXT("레벨업 UI 오픈 — 후보 %d종"), WeaponCards.Num());
}

void ARSPlayerController::OnWeaponSelectCompleted()
{
	GET_GI_SUBSYSTEM_FROM(UUIManagerSubsystem, UMS, GetGameInstance());

	// 위젯이 열려있는 상태에서 호출되므로 IsOpen() 통과 — 캐시에서 반환
	if (ULevelUpWeaponSelectWidget* Widget = Cast<ULevelUpWeaponSelectWidget>(UMS->OpenUIByID(EUIID::LEVEL_UP)))
	{
		Widget->OnWeaponSelectCompletedDel.RemoveDynamic(this, &ARSPlayerController::OnWeaponSelectCompleted);
	}
	UMS->CloseUIByID(EUIID::LEVEL_UP);

	// 게임 재개 (추후 StageSystem 위임으로 교체 예정)
	if (UWorld* World = GetWorld())
	{
		World->GetWorldSettings()->TimeDilation = 1.f;
	}

	GET_GI_SUBSYSTEM_FROM(ULevelUpSubsystem, LevelUpSys, GetGameInstance());
	LevelUpSys->NotifyWeaponSelectCompleted();

	KHS_INFO(TEXT("레벨업 UI 종료 — 게임 재개"));
}

// ============================================================================
// 무기 교체 UI 관리
// ============================================================================

void ARSPlayerController::OnWeaponSlotFull(FName PendingWeaponID)
{
	GET_GI_SUBSYSTEM_FROM(UUIManagerSubsystem, UMS, GetGameInstance());

	// UIManagerSettings WEAPON_REPLACE 매핑으로 위젯 오픈 — 클래스 프로퍼티 직접 참조 제거
	UWeaponReplaceWidget* Widget = Cast<UWeaponReplaceWidget>(UMS->OpenUIByID(EUIID::WEAPON_REPLACE));
	if (!ensureMsgf(Widget, TEXT("WeaponReplaceWidget 오픈 실패 — UIManagerSettings WEAPON_REPLACE 매핑 확인 필요")))
	{
		GET_GI_SUBSYSTEM_FROM(ULevelUpSubsystem, LevelUpSys, GetGameInstance());
		LevelUpSys->NotifyWeaponSelectCompleted();
		return;
	}

	Widget->OnReplaceCompletedDel.AddDynamic(this, &ARSPlayerController::OnWeaponReplaceCompleted);
	Widget->InitWidget(PendingWeaponID);

	KHS_INFO(TEXT("교체 UI 오픈 — PendingWeaponID: %s"), *PendingWeaponID.ToString());
}

void ARSPlayerController::OnWeaponReplaceCompleted()
{
	GET_GI_SUBSYSTEM_FROM(UUIManagerSubsystem, UMS, GetGameInstance());

	// 위젯이 열려있는 상태에서 호출되므로 IsOpen() 통과 — 캐시에서 반환
	if (UWeaponReplaceWidget* Widget = Cast<UWeaponReplaceWidget>(UMS->OpenUIByID(EUIID::WEAPON_REPLACE)))
	{
		Widget->OnReplaceCompletedDel.RemoveDynamic(this, &ARSPlayerController::OnWeaponReplaceCompleted);
	}
	UMS->CloseUIByID(EUIID::WEAPON_REPLACE);

	// CHG: 교체 UI 완료 후 게임 재개 — LevelUpWeaponSelectWidget와 동일한 복구 경로
	if (UWorld* World = GetWorld())
	{
		World->GetWorldSettings()->TimeDilation = 1.f;
	}

	GET_GI_SUBSYSTEM_FROM(ULevelUpSubsystem, LevelUpSys, GetGameInstance());
	LevelUpSys->NotifyWeaponSelectCompleted();

	KHS_INFO(TEXT("교체 UI 종료 — 게임 재개"));
}

// ============================================================================
// FloatingDamageWidget 풀 — PoolingSubsystem 위임
// ============================================================================

void ARSPlayerController::SpawnFloatingDamage(FVector WorldPos, float Damage)
{
	if (!FloatingDamageWidgetClass)
	{
		KHS_WARN(TEXT("FloatingDamageWidgetClass가 할당되지 않았습니다."));
		return;
	}

	// 화면 밖 방어 — 스크린 변환 실패 시 풀 접근 자체를 건너뜀
	FVector2D ScreenPos;
	if (!ProjectWorldLocationToScreen(WorldPos, ScreenPos, true)) { return; }

	GET_WORLD_SUBSYSTEM(UPoolingSubsystem, PoolSys);
	UFloatingDamageWidget* Widget = Cast<UFloatingDamageWidget>(
		PoolSys->SpawnPooledWidget(FloatingDamageWidgetClass, this));

	if (!Widget)
	{
		KHS_WARN(TEXT("FloatingDamageWidget 풀 획득 실패"));
		return;
	}

	Widget->SetPositionInViewport(ScreenPos, true);  // bRemoveDPIScale=true: 픽셀→Slate 단위 변환

	if (!Widget->IsInViewport())
	{
		Widget->AddToViewport(UUIManagerSubsystem::ZOrder_PAGE);
	}

	Widget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	Widget->PlayFloatAnimation(Damage);
}

void ARSPlayerController::ReturnFloatingDamageToPool(UFloatingDamageWidget* Widget)
{
	if (!Widget) { return; }

	Widget->RemoveFromParent();

	GET_WORLD_SUBSYSTEM(UPoolingSubsystem, PoolSys);
	PoolSys->ReturnWidgetToPool(Widget);
}
