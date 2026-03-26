// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/RSPlayerCharacter.h"
#include "RoastStaffGAS.h"
#include "System/LoggingSystem.h"
#include "Subsystems/EquipmentSubsystem.h"
#include "Subsystems/LevelUpSubsystem.h"
#include "Character/Player/RSPlayerState.h"
#include "Component/EquipmentComponent.h"
#include "GAS/Attributes/PlayerAttributeSet.h"
#include "AbilitySystemComponent.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/SpringArmComponent.h"
#include "TimerManager.h"

ARSPlayerCharacter::ARSPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->bUsePawnControlRotation = false;
	SpringArm->SetRelativeRotation(FRotator(-50.f, 0.f, 0.f));
	SpringArm->TargetArmLength = 2200.f;
	SpringArm->bDoCollisionTest = false;
	SpringArm->bInheritYaw = false;
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 0.5f;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
	Camera->SetFieldOfView(75.f);
	
	GetCharacterMovement()->GravityScale = 1.5f;
	GetCharacterMovement()->MaxAcceleration = 1000.0f;
	GetCharacterMovement()->bCanWalkOffLedges = false;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 640.0f, 0.0f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;
	
	EquipmentComp = CreateDefaultSubobject<UEquipmentComponent>(TEXT("EquipmentComp"));
}

void ARSPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	InitializeAbilitySystem();
}


UAbilitySystemComponent* ARSPlayerCharacter::GetAbilitySystemComponent() const
{
	const ARSPlayerState* PS = GetRSPlayerState();
	if (!PS)
	{
		KHS_WARN(TEXT("PlayerState IS NULL."));
		return nullptr;;
	}
	
	return PS->GetAbilitySystemComponent();
}

UBaseAttributeSet* ARSPlayerCharacter::GetBaseAttributeSet() const
{
	const ARSPlayerState* PS = GetRSPlayerState();
	if (!PS)
	{
		KHS_WARN(TEXT("PlayerState IS NULL."));
		return nullptr;;
	}
	
	return PS->GetPlayerAttributeSet();
}

void ARSPlayerCharacter::InitializeAbilitySystem()
{
	ARSPlayerState* PS = GetRSPlayerState();
	if (!ensureMsgf(PS, TEXT("PS is null")))
	{
		return;
	}

	// PlayerState에 Owner/Avatar 세팅 및 스탯 주입 위임
	PS->InitializeAbilitySystem(this);

	// AS 델리게이트 바인딩
	BindAttributeDelegates();

	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ensureMsgf(ASC, TEXT("ASC is null")))
	{
		return;
	}
	
	GET_GI_SUBSYSTEM(UEquipmentSubsystem, EquipSys);
	EquipSys->InitializeSubsystem(ASC);

	GET_GI_SUBSYSTEM(ULevelUpSubsystem, LevelUpSys);
	LevelUpSys->InitializeSubsystem(ASC, PS->GetPlayerAttributeSet(), AddEXPEffectClass);

	// FloatingDamageWidget 구독 (BaseCharacter 공통)
	SetupDamageDelegate();

	KHS_INFO(TEXT("GAS 초기화 완료."));
}

void ARSPlayerCharacter::HandleDeath()
{
	// 공통 사망 프로세스 (GA 종료, GE 제거, 충돌 비활성화)
	Super::HandleDeath();

	// 플레이어 고유 사망 처리
	//입력 비활성화
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
	{
		KHS_WARN(TEXT("PlayerController is NULL."));
		return;
	}
	
	PC->SetIgnoreMoveInput(true);
	PC->SetIgnoreLookInput(true);

	// 무기 발사 모두 중지
	GET_GI_SUBSYSTEM(UEquipmentSubsystem, EquipSys);
	EquipSys->StopAllFire();

	// TODO : 스테이지 시스템 생기면 사망 이벤트 전달
	// TODO : 결과 화면 전환

	KHS_INFO(TEXT("플레이어 고유 사망 처리 완료."));
}

ARSPlayerState* ARSPlayerCharacter::GetRSPlayerState() const
{
	return GetPlayerState<ARSPlayerState>();
}
