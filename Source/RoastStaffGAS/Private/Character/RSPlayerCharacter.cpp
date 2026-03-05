// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/RSPlayerCharacter.h"
#include "Character/RSPlayerState.h"
#include "Component/EquipmentComponent.h"
#include "GAS/Attributes/PlayerAttributeSet.h"
#include "GAS/Tags/RSGameplayTags.h"
#include "System/LoggingSystem.h"

#include "AbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "TimerManager.h"
#include "GameFramework/CharacterMovementComponent.h"

ARSPlayerCharacter::ARSPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

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

void ARSPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
	{
		KHS_WARN(TEXT("PlayerController is NULL"));
	}
	
	UEnhancedInputLocalPlayerSubsystem* Subsys = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
	if (!Subsys)
	{
		KHS_WARN(TEXT("EnhancedInputSubsystem is NULL"));
	}
	
	if (IMC)
	{
		Subsys->AddMappingContext(IMC, 0);
	}
}

void ARSPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	InitializeAbilitySystem();
	
	// [임시 테스트 코드 — 이후 레벨업 시스템으로 교체]
    if (EquipmentComp)
    {
        EquipmentComp->EquipWeapon(FName("WPN_FIRESTAFF_Lv1"));
        EquipmentComp->EquipWeapon(FName("WPN_FIRESTAFF_Lv1"));
        EquipmentComp->EquipWeapon(FName("WPN_FIRESTAFF_Lv1"));
    }
}

bool ARSPlayerCharacter::HandleMouseAim()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
	{
		KHS_WARN(TEXT("PlayerController is NULL."));
		return false;
	}
	
	FHitResult HitResult;
	PC->GetHitResultUnderCursor(ECC_Visibility, false, HitResult);

	if (HitResult.bBlockingHit)
	{
		const FRotator LookAt =	(HitResult.Location - GetActorLocation()).Rotation();
		AimAngle = LookAt.Yaw;
		SetActorRotation(FRotator(0.f, AimAngle, 0.f));
	}
	return true;
}

void ARSPlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	// 쿼터뷰 — 마우스 커서 방향으로 캐릭터 회전
	if (!HandleMouseAim())
	{
		return;
	}
}

void ARSPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EIC = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);

	EIC->BindAction(IA_Move,  ETriggerEvent::Triggered, this, &ARSPlayerCharacter::OnMove);
	EIC->BindAction(IA_MouseAim, ETriggerEvent::Triggered, this, &ARSPlayerCharacter::OnMouseAim);
	EIC->BindAction(IA_Dash,  ETriggerEvent::Triggered, this, &ARSPlayerCharacter::OnDash);
	EIC->BindAction(IA_Attack, ETriggerEvent::Started,   this, &ARSPlayerCharacter::OnShootStart);
	EIC->BindAction(IA_Slot1, ETriggerEvent::Started, this, &ARSPlayerCharacter::OnSlotActivate1);
	EIC->BindAction(IA_Slot2, ETriggerEvent::Started, this, &ARSPlayerCharacter::OnSlotActivate2);
	EIC->BindAction(IA_Slot3, ETriggerEvent::Started, this, &ARSPlayerCharacter::OnSlotActivate3);
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
	EquipmentComp->InitializeWithASC(ASC);
	
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
	if (EquipmentComp)
	{
		EquipmentComp->StopAllFire();
	}

	// TODO : 스테이지 시스템 생기면 사망 이벤트 전달
	// TODO : 결과 화면 전환

	KHS_INFO(TEXT("플레이어 고유 사망 처리 완료."));
}

void ARSPlayerCharacter::TryActivateDash()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC)
	{
		KHS_WARN(TEXT("ASC IS NULL"));
		return;
	}

	FGameplayTagContainer TempTags;
	TempTags.AddTag(RSTags::Ability_Movement_Dash);
	ASC->TryActivateAbilitiesByTag(TempTags);
}

ARSPlayerState* ARSPlayerCharacter::GetRSPlayerState() const
{
	return GetPlayerState<ARSPlayerState>();
}

void ARSPlayerCharacter::OnMove(const FInputActionValue& Value)
{
	const FVector2D Input = Value.Get<FVector2D>();
	LastMoveInput = Input;

	AddMovementInput(FVector::ForwardVector, Input.X);
	AddMovementInput(FVector::RightVector,   Input.Y);
}

void ARSPlayerCharacter::OnMouseAim(const FInputActionValue& Value)
{
	FVector2D inputVector = Value.Get<FVector2D>();
	
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
	{
		KHS_WARN(TEXT("PlayerController is NULL."));
		return;
	}
	PC->SetShowMouseCursor(true);
	
	AimAngle = FMath::RadiansToDegrees(FMath::Atan2(inputVector.Y, inputVector.X));
}

void ARSPlayerCharacter::OnDash(const FInputActionValue& Value)
{
	TryActivateDash();
}

void ARSPlayerCharacter::OnShootStart(const FInputActionValue& Value)
{
	if (!ensureMsgf(EquipmentComp, TEXT("EquipmentComp is null")))
	{
		return;
	}

	EquipmentComp->OnAttackInput();
}


void ARSPlayerCharacter::OnSlotActivate1(const FInputActionValue& Value)
{
	if (!ensureMsgf(EquipmentComp, TEXT("EquipmentComp is null")))
	{
		return;
	}

	EquipmentComp->RequestSlotActivate(0);
}

void ARSPlayerCharacter::OnSlotActivate2(const FInputActionValue& Value)
{
	if (!ensureMsgf(EquipmentComp, TEXT("EquipmentComp is null")))
	{
		return;
	}

	EquipmentComp->RequestSlotActivate(1);
}

void ARSPlayerCharacter::OnSlotActivate3(const FInputActionValue& Value)
{
	if (!ensureMsgf(EquipmentComp, TEXT("EquipmentComp is null")))
	{
		return;
	}

	EquipmentComp->RequestSlotActivate(2);
}
