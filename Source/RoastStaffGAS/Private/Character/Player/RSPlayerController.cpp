// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/RSPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

#include "RoastStaffGAS.h"
#include "Subsystems/EquipmentSubsystem.h"
#include "Subsystems/UIManagerSubsystem.h"
#include "UI/RSHUDWidget.h"
#include "UI/WeaponSlotContainerWidget.h"
#include "UI/WeaponSlotWidget.h"
#include "UI/FloatingDamageWidget.h"
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

	//슬롯 델리게이트 구독
	GET_GI_SUBSYSTEM_FROM(UUIManagerSubsystem, UMS, GetGameInstance());
	GET_GI_SUBSYSTEM_FROM(UEquipmentSubsystem, EquipSys, GetGameInstance());
	
	EquipSys->OnSlotUpdatedDel.AddDynamic(this, &ARSPlayerController::OnSlotUpdated);
	KHS_INFO(TEXT("OnSlotUpdatedDel 구독 완료 "));

	//HUD UI오픈
	if (!HUDWidgetClass)
	{
		KHS_WARN(TEXT("HUD WBP 미할당"));
		return;
	}
	
	CachedHUDUI = UMS->OpenUI<URSHUDWidget>(HUDWidgetClass);

	if (!CachedHUDUI)
	{
		KHS_WARN(TEXT("HUD Widget 생성 실패"));
		return;
	}

	PrewarmFloatingDamagePool();
}

void ARSPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GET_GI_SUBSYSTEM_FROM(UEquipmentSubsystem, EquipSys, GetGameInstance());
	EquipSys->OnSlotUpdatedDel.RemoveDynamic(this, &ARSPlayerController::OnSlotUpdated);
	
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

	UWeaponSlotContainerWidget* SlotContainer = CachedHUDUI->GetSlotContainerWidget();
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
// FloatingDamageWidget 풀
// ============================================================================

void ARSPlayerController::PrewarmFloatingDamagePool()
{
	if (!FloatingDamageWidgetClass)
	{
		KHS_WARN(TEXT("FloatingDamageWidgetClass가 할당되지 않아 prewarm을 건너뜁니다."));
		return;
	}

	const int32 PrewarmCount = FMath::Min(PoolInitialSize, PoolMaxSize);
	for (int32 i = 0; i < PrewarmCount; ++i)
	{
		UFloatingDamageWidget* Widget = CreateWidget<UFloatingDamageWidget>(this, FloatingDamageWidgetClass);
		if (Widget)
		{
			Widget->SetVisibility(ESlateVisibility::Collapsed);
			FloatingDamagePool.Add(Widget);
			++TotalCreatedCount;
		}
	}

	KHS_INFO(TEXT("FloatingDamageWidget prewarm 완료: %d개"), TotalCreatedCount);
}

void ARSPlayerController::SpawnFloatingDamage(FVector WorldPos, float Damage)
{
	if (!FloatingDamageWidgetClass)
	{
		KHS_WARN(TEXT("FloatingDamageWidgetClass가 할당되지 않았습니다."));
		return;
	}

	UFloatingDamageWidget* Widget = nullptr;

	if (FloatingDamagePool.Num() > 0)
	{
		Widget = FloatingDamagePool.Pop();
	}
	else if (TotalCreatedCount < PoolMaxSize)
	{
		Widget = CreateWidget<UFloatingDamageWidget>(this, FloatingDamageWidgetClass);
		if (Widget)
		{
			++TotalCreatedCount;
		}
	}
	else
	{
		KHS_WARN(TEXT("FloatingDamageWidget 풀 상한(%d) 초과 — 스폰 스킵"), PoolMaxSize);
		return;
	}

	if (!Widget)
	{
		KHS_WARN(TEXT("FloatingDamageWidget 생성 실패"));
		return;
	}

	// 화면 밖 방어 — 스크린 변환 실패 시 풀 반납 후 반환
	FVector2D ScreenPos;
	if (!ProjectWorldLocationToScreen(WorldPos, ScreenPos, true))
	{
		FloatingDamagePool.Add(Widget);
		return;
	}

	Widget->SetPositionInViewport(ScreenPos, true);  // bRemoveDPIScale=true: 픽셀→Slate 단위 변환

	if (!Widget->IsInViewport())
	{
		Widget->AddToViewport(200);
	}

	Widget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	Widget->PlayFloatAnimation(Damage);
}

void ARSPlayerController::ReturnFloatingDamageToPool(UFloatingDamageWidget* Widget)
{
	if (!Widget)
	{
		return;
	}

	Widget->RemoveFromParent();
	Widget->SetVisibility(ESlateVisibility::Collapsed);
	FloatingDamagePool.Add(Widget);
}
