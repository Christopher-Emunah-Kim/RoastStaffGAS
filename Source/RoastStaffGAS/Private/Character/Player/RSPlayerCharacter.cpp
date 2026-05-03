// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/RSPlayerCharacter.h"
#include "RoastStaffGAS.h"
#include "System/LoggingSystem.h"
#include "Subsystems/EquipmentSubsystem.h"
#include "Subsystems/LevelUpSubsystem.h"
#include "Subsystems/SkillManagerSubsystem.h"
#include "Subsystems/PassiveSlotSubsystem.h"
#include "Subsystems/SaveGameSubsystem.h"
#include "Character/Player/RSPlayerState.h"
#include "Component/EquipmentComponent.h"
#include "GAS/Attributes/PlayerAttributeSet.h"
#include "Core/RSGameMode.h"
#include "AbilitySystemComponent.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/SpringArmComponent.h"
#include "TimerManager.h"

ARSPlayerCharacter::ARSPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	// 에너미 투사체 피격 허용 — EnemyProjectile 채널 Overlap
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Overlap);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->bUsePawnControlRotation = false;
	SpringArm->SetRelativeRotation(FRotator(-50.f, 0.f, 0.f));
	SpringArm->TargetArmLength = 1900.f;
	SpringArm->bDoCollisionTest = false;
	SpringArm->bInheritYaw = false;
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 8.f;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
	Camera->SetFieldOfView(75.f);
	
	GetCharacterMovement()->GravityScale = 1.5f;
	GetCharacterMovement()->MaxAcceleration = 1000.0f;
	GetCharacterMovement()->bCanWalkOffLedges = false;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 640.0f, 0.0f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;
	GetCharacterMovement()->bAllowPhysicsRotationDuringAnimRootMotion = true;
	
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
	
	GET_GI_SUBSYSTEM(UEquipmentSubsystem, EquipSys)
	EquipSys->InitializeSubsystem(ASC);

	GET_GI_SUBSYSTEM(ULevelUpSubsystem, LevelUpSys)
	LevelUpSys->InitializeSubsystem(ASC, PS->GetPlayerAttributeSet(), AddEXPEffectClass);

	GET_GI_SUBSYSTEM(USaveGameSubsystem, SGS)
	const FName CharID = SGS->GetLastSelectedCharacter();

	GET_WORLD_SUBSYSTEM(USkillManagerSubsystem, SkillMgr)
	SkillMgr->InitializeSkills(CharID, ASC);
	
	GET_WORLD_SUBSYSTEM(UPassiveSlotSubsystem, PassiveSys)
	PassiveSys->InitializeSubsystem(ASC);

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

	// 스테이지 실패 처리 (GameMode에 전달)
	ARSGameMode* GM = GetWorld()->GetAuthGameMode<ARSGameMode>();
	if (GM)
	{
		GM->OnStageFailed();
	}

	KHS_INFO(TEXT("플레이어 고유 사망 처리 완료."));
}

ARSPlayerState* ARSPlayerCharacter::GetRSPlayerState() const
{
	return GetPlayerState<ARSPlayerState>();
}
