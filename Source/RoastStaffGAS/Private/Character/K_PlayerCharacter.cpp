// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/K_PlayerCharacter.h"

#include "AbilitySystem/Attributes/K_BaseAttributeSet.h"
#include "Character/K_PlayerController.h"
#include "System/K_LoggingSystem.h"

#include "TwinStickProjectile.h"

#include "EnhancedInputComponent.h"
#include "AbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"


AK_PlayerCharacter::AK_PlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	
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
	
	GetCharacterMovement()->GravityScale = 1.5f;
	GetCharacterMovement()->MaxAcceleration = 1000.0f;
	GetCharacterMovement()->BrakingFrictionFactor = 1.0f;
	GetCharacterMovement()->bCanWalkOffLedges = false;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 640.0f, 0.0f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;
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
	
	ASC->SetNumericAttributeBase(BaseAttributeSet->GetMaxHealthAttribute(), 200.f);
	ASC->SetNumericAttributeBase(BaseAttributeSet->GetMaxManaAttribute(), 200.f);
}

void AK_PlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
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
	FVector LaunchDir = FVector::ZeroVector;

	LaunchDir.X = FMath::Clamp(LastMoveInput.X, -1.0f, 1.0f);
	LaunchDir.Y = FMath::Clamp(LastMoveInput.Y, -1.0f, 1.0f);

	LaunchCharacter(LaunchDir * DashImpulse, true, true);
}

void AK_PlayerCharacter::OnShootStart(const FInputActionValue& Value)
{
	DoShoot();
	GetWorld()->GetTimerManager().SetTimer(AutoFireTimer, this, &AK_PlayerCharacter::DoShoot, AutoFireDelay, true);
}

void AK_PlayerCharacter::OnShootStop(const FInputActionValue& Value)
{
	GetWorld()->GetTimerManager().ClearTimer(AutoFireTimer);
}

void AK_PlayerCharacter::OnFireballAttack(const FInputActionValue& Value)
{
	// const float gameTime = GetWorld()->GetTimeSeconds();
	//
	// if (gameTime - LastFireballAttackTime > FireballCooldownTime)
	// {
	// 	LastFireballAttackTime = gameTime;
	// 	
	// 	ATwinStickAoEAttack* AoE = GetWorld()->SpawnActor<ATwinStickAoEAttack>(FireballAttackClass, GetActorTransform());
	// }
	
	if (!ASC || !FireballAbilityClass)
	{
		KHS_WARN(TEXT("ASC or FireballAbilityClass is not valid"));
	}
	
	bool bSuccess = ASC->TryActivateAbilityByClass(FireballAbilityClass);
	if (!bSuccess)
	{
		KHS_WARN(TEXT("Can't activate Ability for Fireball"));
	}
}

void AK_PlayerCharacter::DoShoot()
{
	FTransform ProjectileTransform = GetActorTransform();

	// apply the projectile spawn offset
	FVector ProjectileLocation = ProjectileTransform.GetLocation() + ProjectileTransform.GetRotation().RotateVector(FVector::ForwardVector * ProjectileOffset);
	ProjectileTransform.SetLocation(ProjectileLocation);

	ATwinStickProjectile* Projectile = GetWorld()->SpawnActor<ATwinStickProjectile>(ProjectileClass, ProjectileTransform);
}

