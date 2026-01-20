// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/K_PlayerCharacter.h"
#include "Character/K_PlayerController.h"
#include "Character/K_PlayerState.h"
#include "System/K_LoggingSystem.h"
#include "AbilitySystem/Attributes/K_BaseAttributeSet.h"
#include "AbilitySystem/GameplayTags/K_GameplayTags.h"

#include "EnhancedInputComponent.h"
#include "AbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"


AK_PlayerCharacter::AK_PlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	
	//Camera Setup
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->SetRelativeRotation(FRotator(-50.f, 0.f, 0.f));
	SpringArm->TargetArmLength = 2200.f;
	SpringArm->bDoCollisionTest = false;
	SpringArm->bInheritYaw = false;
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 0.5f;
	
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
	Camera->SetFieldOfView(75.f);
	
	//Movement Setup
	GetCharacterMovement()->GravityScale = 1.5f;
	GetCharacterMovement()->MaxAcceleration = 1000.0f;
	GetCharacterMovement()->BrakingFrictionFactor = 1.0f;
	GetCharacterMovement()->bCanWalkOffLedges = false;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 640.0f, 0.0f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;
	
	// NOTE: BaseCharacter 생성자에서 ASC와 BaseAttributeSet이 생성되지만,
	// PlayerCharacter는 이들을 사용하지 않고 PlayerState의 것을 사용함.
	// 이는 약간의 메모리 낭비지만, 코드 단순화를 위해 허용.
	// 최적화가 필요하다면 BaseCharacter에서 조건부 생성하도록 수정 예정.
}

void AK_PlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	AK_PlayerController* pc = CastChecked<AK_PlayerController>(GetWorld()->GetFirstPlayerController());
	KPlayerController = pc;
	KPlayerController->SetInputMode(FInputModeGameOnly());
}

void AK_PlayerCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	GetWorld()->GetTimerManager().ClearTimer(AutoFireTimer);
}

void AK_PlayerCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	//NOTE - 이 시점엔 Playerstate가 미생성 가능성 있음. 
	//ASC기본설정은 이후에 처리.
}

void AK_PlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController); //PlayerState설정 시점
	
	InitializeAbilitySystem(); //Server는 PossessedBy가 호출되므로 여기서 초기화
	//KHS_INFO(TEXT("[PlayerCharacter] PossessedBy called on Server"));
}

void AK_PlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	
	InitializeAbilitySystem(); //클라는 PlyaerState리플리케이트 된 이후 초기화
	//KHS_INFO(TEXT("[PlayerCharacter] OnRep_PlayerState called on Client"));
}

void AK_PlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	//쿼터뷰 - 마우스에임 따라 회전하도록 설정
	const FRotator oldRot = GetActorRotation();
	
	if (KPlayerController)
	{
		//마우스 커서 위치 
		FHitResult outHit;
		KPlayerController->GetHitResultUnderCursorByChannel(MouseAimTraceChannel, true, outHit);
		//Aim 회전값
		const FRotator newRot = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), outHit.Location);
		//AimAngle에 캐싱
		AimAngle = newRot.Yaw;
		//Yaw값 업데이트
		SetActorRotation(FRotator(oldRot.Pitch, AimAngle, oldRot.Roll));
	}
	
}

void AK_PlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* enhanced = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		enhanced->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AK_PlayerCharacter::OnMove);
		enhanced->BindAction(IA_MouseAim, ETriggerEvent::Triggered, this, &AK_PlayerCharacter::OnMouseAim);
		enhanced->BindAction(IA_Dash, ETriggerEvent::Triggered, this, &AK_PlayerCharacter::OnDash);
		
		enhanced->BindAction(IA_Shoot, ETriggerEvent::Started, this, &AK_PlayerCharacter::OnShootStart);
		enhanced->BindAction(IA_Shoot, ETriggerEvent::Completed, this, &AK_PlayerCharacter::OnShootStop);
		enhanced->BindAction(IA_Shoot, ETriggerEvent::Canceled, this, &AK_PlayerCharacter::OnShootStop);
		enhanced->BindAction(IA_FireBall, ETriggerEvent::Triggered, this, &AK_PlayerCharacter::OnFireballAttack);
	}
}

UAbilitySystemComponent* AK_PlayerCharacter::GetAbilitySystemComponent() const
{
	const AK_PlayerState* ps = GetKPlayerState();
	return ps? ps->GetAbilitySystemComponent() : nullptr;
}

UK_BaseAttributeSet* AK_PlayerCharacter::GetAttributeSet() const
{
	const AK_PlayerState* ps = GetKPlayerState();
	return ps?ps->GetBaseAttributeSet() : nullptr;
}

void AK_PlayerCharacter::InitializeAbilitySystem()
{
	// PlayerState에서 ASC 초기화 위임
	AK_PlayerState* PS = GetKPlayerState();
	if (!PS)
	{
		KHS_WARN(TEXT("[PlayerCharacter] PlayerState not found, cannot initialize abilities"));
		return;
	}

	// PlayerState에게 초기화 요청
	// Owner: PlayerState, Avatar: this (PlayerCharacter)
	PS->InitializeAbilities(this);

	// Attribute 기본값 설정 (PlayerState의 ASC 사용)
	UAbilitySystemComponent* AbilitySystem = PS->GetAbilitySystemComponent();
	UK_BaseAttributeSet* AttributeSet = PS->GetBaseAttributeSet();

	if (AbilitySystem && AttributeSet)
	{
		AbilitySystem->SetNumericAttributeBase(AttributeSet->GetMaxHealthAttribute(), 200.f);
		AbilitySystem->SetNumericAttributeBase(AttributeSet->GetHealthAttribute(), 200.f);
		AbilitySystem->SetNumericAttributeBase(AttributeSet->GetMaxManaAttribute(), 200.f);
		AbilitySystem->SetNumericAttributeBase(AttributeSet->GetManaAttribute(), 200.f);
	}

	// 부모 클래스의 플래그 설정 (중복 방지용)
	bASCInitialized = true;
	
	//KHS_INFO(TEXT("[PlayerCharacter] AbilitySystem initialized via PlayerState"));
}


void AK_PlayerCharacter::OnMove(const FInputActionValue& Value)
{
	FVector2D inputVector = Value.Get<FVector2D>();
	
	LastMoveInput = inputVector;
	
	FRotator flatRot = GetControlRotation();
	flatRot.Pitch = 0.f;
	
	AddMovementInput(flatRot.RotateVector(FVector::ForwardVector), LastMoveInput.X);
	AddMovementInput(flatRot.RotateVector(FVector::RightVector), LastMoveInput.Y);
}

void AK_PlayerCharacter::OnMouseAim(const FInputActionValue& Value)
{
	FVector2D inputVector = Value.Get<FVector2D>();
	
	check(KPlayerController);
	KPlayerController->SetShowMouseCursor(true);
	
	AimAngle = FMath::RadiansToDegrees(FMath::Atan2(inputVector.Y, inputVector.X));
}

void AK_PlayerCharacter::OnDash(const FInputActionValue& Value)
{
	TryActivateDash();
}

void AK_PlayerCharacter::OnShootStart(const FInputActionValue& Value)
{
	TryActivateBasicShoot();
	GetWorld()->GetTimerManager().SetTimer(AutoFireTimer, this, &AK_PlayerCharacter::TryActivateBasicShoot, AutoFireDelay, true);
}

void AK_PlayerCharacter::OnShootStop(const FInputActionValue& Value)
{
	GetWorld()->GetTimerManager().ClearTimer(AutoFireTimer);
}

void AK_PlayerCharacter::OnFireballAttack(const FInputActionValue& Value)
{
	TryActivateFireball();
}

void AK_PlayerCharacter::TryActivateDash()
{
	UAbilitySystemComponent* abilityComp = GetAbilitySystemComponent();
	
	if (!abilityComp)
	{
		KHS_WARN(TEXT("ASC is not valid"));
		return;
	}
	
	//GameplayTag기반 능력 발동. 
	//PlayerState의 InitialAbilities에 GA가 등록되어있어야함.
	FGameplayTagContainer dashTags;
	dashTags.AddTag(KTags::Ability_Movement_Dash);
	bool bSuccess = abilityComp->TryActivateAbilitiesByTag(dashTags);
	
	if (!bSuccess)
	{
		KHS_INFO(TEXT("[PlyaerCharacter] BasicShoot activation failed"));
	}
}

void AK_PlayerCharacter::TryActivateBasicShoot()
{
	UAbilitySystemComponent* abilityComp = GetAbilitySystemComponent();
	
	if (!abilityComp)
	{
		KHS_WARN(TEXT("ASC is not valid"));
		return;
	}
	
	//GameplayTag기반 능력 발동. 
	//PlayerState의 InitialAbilities에 GA가 등록되어있어야함.
	FGameplayTagContainer basicShootTags;
	basicShootTags.AddTag(KTags::Ability_Combat_BasicShoot);
	bool bSuccess = abilityComp->TryActivateAbilitiesByTag(basicShootTags);
	
	if (!bSuccess)
	{
		KHS_INFO(TEXT("[PlyaerCharacter] BasicShoot activation failed"));
	}
}

void AK_PlayerCharacter::TryActivateFireball()
{
	UAbilitySystemComponent* abilityComp = GetAbilitySystemComponent();
	
	if (!abilityComp)
	{
		KHS_WARN(TEXT("ASC is not valid"));
		return;
	}
	
	//GameplayTag기반 능력 발동. 
	//PlayerState의 InitialAbilities에 GA가 등록되어있어야함.
	FGameplayTagContainer fireballTag;
	fireballTag.AddTag(KTags::Ability_Skill_Fireball);
	bool bSuccess = abilityComp->TryActivateAbilitiesByTag(fireballTag);
	
	if (!bSuccess)
	{
		KHS_INFO(TEXT("[PlayerCharacter] Fireball activation failed"));
	}
}


AK_PlayerState* AK_PlayerCharacter::GetKPlayerState() const
{
	return GetPlayerState<AK_PlayerState>();
}

FVector AK_PlayerCharacter::GetDashDirection() const
{
	if (LastMoveInput.IsZero())
	{
		return GetActorForwardVector();
	}
	
	FVector dashDirection = FVector(LastMoveInput.X, LastMoveInput.Y, 0.0f);
	
	return dashDirection.GetSafeNormal();
}

