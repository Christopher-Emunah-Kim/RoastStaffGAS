// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/K_EnemyCharacter.h"

#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "AbilitySystem/Attributes/K_BaseAttributeSet.h"
#include "AbilitySystem/GameplayTags/K_GameplayTags.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "System/K_LoggingSystem.h"

AK_EnemyCharacter::AK_EnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	
	//WidgetCmp
	StateWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("StateWidgetComp"));
	StateWidgetComp->SetupAttachment(RootComponent);
	StateWidgetComp->SetRelativeLocation(FVector(0.f, 0.f, 100.f));
	StateWidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
	StateWidgetComp->SetDrawSize(FVector2D(150.f, 20.f));
	StateWidgetComp->SetVisibility(false);
}

void AK_EnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AK_EnemyCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	//타이머 해제
	GetWorld()->GetTimerManager().ClearTimer(DamageFeedbackTimer);
	GetWorld()->GetTimerManager().ClearTimer(DeathDestroyTimer);
	
	
	
	Super::EndPlay(EndPlayReason);
}

void AK_EnemyCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	if (ASC && BaseAttributeSet)
	{
		ASC->SetNumericAttributeBase(BaseAttributeSet->GetMaxHealthAttribute(), 50.f);
		ASC->SetNumericAttributeBase(BaseAttributeSet->GetHealthAttribute(), 50.f);
		
		ASC->SetNumericAttributeBase(BaseAttributeSet->GetMaxManaAttribute(), 0.f);
		ASC->SetNumericAttributeBase(BaseAttributeSet->GetManaAttribute(), 0.f);
	}
	
	InitializeStateWidget();
}

void AK_EnemyCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if (!bIsDead)
	{
		TickSimpleAI(DeltaSeconds);
	}
}

void AK_EnemyCharacter::InitializeAbilitySystem()
{
	Super::InitializeAbilitySystem();
	
	check(ASC);
	
	ASC->GenericGameplayEventCallbacks.FindOrAdd(KTags::Event_Combat_Death).AddUObject(this, &AK_EnemyCharacter::OnDeathEvent);
	ASC->GenericGameplayEventCallbacks.FindOrAdd(KTags::Event_Combat_TakeDamage).AddUObject(this, &AK_EnemyCharacter::OnTakeDamageEvent);
	
	KHS_SCREEN_INFO(TEXT("[%s] Enemy ASC initialized, event callbacks registered"), *GetName());
}

void AK_EnemyCharacter::TickSimpleAI(float DeltaTime)
{
	APawn* player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!ensureMsgf(player, TEXT("Invalid Player")))
	{
		return;
	}
	
	const float distance = FVector::Dist(GetActorLocation(), player->GetActorLocation());
	
	if (distance <= DetectionRange && distance > 150.f)
	{
		FVector direction = (player->GetActorLocation() - GetActorLocation()).GetSafeNormal();
		direction.Z = 0.f;
		
		if (!direction.IsNearlyZero())
		{
			FRotator targetRot = direction.Rotation();
			SetActorRotation(FMath::RInterpTo(GetActorRotation(), targetRot, DeltaTime, 5.f));
		}
		
		AddMovementInput(direction, 1.f);
	}
}

void AK_EnemyCharacter::OnDeathEvent(const FGameplayEventData* Payload)
{
	//중복 처리 방지
	if (bIsDead)
	{
		return;
	}
	
	bIsDead = true;
	
	KHS_SCREEN_INFO(TEXT("[%s] Death Event recieved! Instigator : %s"), *GetName(), Payload->Instigator? *Payload->Instigator->GetName() : TEXT("NONE"));
	
	HandleDeath();
}

void AK_EnemyCharacter::OnTakeDamageEvent(const FGameplayEventData* Payload)
{
	if (bIsDead)
	{
		return;
	}
	
	KHS_SCREEN_INFO(TEXT("[%s] Damage Taken : %.1f"), *GetName(), Payload->EventMagnitude);
	
	HandleDamageFeedback();
	
	//StateWidget이 숨겨져있었다면표시
	if (StateWidgetComp && !StateWidgetComp->IsVisible())
	{
		StateWidgetComp->SetVisibility(true);
	}
}

void AK_EnemyCharacter::HandleDeath_Implementation()
{
	KHS_INFO(TEXT("[%s] Starting death sequence"), *GetName());
	
	//충돌 비활성화
	UCapsuleComponent* capsule = GetCapsuleComponent();
	if (!ensureMsgf(capsule, TEXT("Invalid Capsulecommp")))
	{
		return;
	}
	capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	//움직임 멈추기
	UCharacterMovementComponent* movementComp = GetCharacterMovement();
	if (!ensureMsgf(movementComp, TEXT("Invalid movementComp")))
	{
		return;
	}
	movementComp->StopMovementImmediately();
	movementComp->DisableMovement();
	
	//ASC태그부여
	check(ASC);
	ASC->CancelAbilities();
	ASC->AddLooseGameplayTag(KTags::State_Dead);
	
	if (StateWidgetComp)
	{
		StateWidgetComp->SetVisibility(false);
	}
	
	//메시 숨기기
	USkeletalMeshComponent* meshComp = GetMesh();
	if (!ensureMsgf(meshComp, TEXT("Invalid meshComp")))
	{
		return;
	}
	meshComp->SetVisibility(false);
	
	//일정 시간 후 파괴
	GetWorld()->GetTimerManager().SetTimer(DeathDestroyTimer, this, &AK_EnemyCharacter::DestroyAfterDeath, DeathDestroyDelay, false);
	
	KHS_INFO(TEXT("[%s] Will be destroyed"), *GetName());
}

void AK_EnemyCharacter::DestroyAfterDeath()
{
	KHS_INFO(TEXT("[%s] Destroying after Death"), *GetName());
	Destroy();
}


void AK_EnemyCharacter::HandleDamageFeedback_Implementation()
{
	USkeletalMeshComponent* meshComp = GetMesh();
	if (!ensureMsgf(meshComp, TEXT("Invalid meshComp")))
	{
		return;
	}
	GetWorld()->GetTimerManager().ClearTimer(DamageFeedbackTimer);
	
	meshComp->SetVisibility(false);
	
	GetWorld()->GetTimerManager().SetTimer(
	   DamageFeedbackTimer,
	   [meshComp]()
	   {
		   if (meshComp)
		   {
			   meshComp->SetVisibility(true);
		   }
	   },
	   DamageFeedbackDuration,
	   false
   );
}

void AK_EnemyCharacter::InitializeStateWidget()
{
	if (!ensureMsgf(StateWidgetClass, TEXT("StateWidgetClass is not Set!")))
	{
		return;
	}
	
	StateWidgetComp->SetWidgetClass(StateWidgetClass);
	StateWidgetComp->InitWidget();
	
	StateWidget = Cast<UEnemyStateWidget>(StateWidgetComp->GetUserWidgetObject());
	
	if (!StateWidget || !ASC)
	{
		KHS_WARN(TEXT("StateWidget and ASC are not valid"));
		return;
	}
	
	StateWidget->BindToASC(ASC);
	KHS_INFO(TEXT("[%s] StateWidget bound to ASC"), *GetName());
}
