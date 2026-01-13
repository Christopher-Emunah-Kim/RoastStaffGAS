// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/K_PlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "Character/K_PlayerCharacter.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "System/K_LoggingSystem.h"

void AK_PlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void AK_PlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	if (!IsLocalPlayerController())
	{
		KHS_WARN(TEXT("로컬 플레이어가 아님"));
	}
	
	UEnhancedInputLocalPlayerSubsystem* subsys = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	check(subsys);
	subsys->AddMappingContext(IMC, 0);
	
}

void AK_PlayerController::OnPossess(APawn* InPawn)
{

	Super::OnPossess(InPawn);
	
	InPawn->OnDestroyed.AddDynamic(this, &AK_PlayerController::OnPawnDestroyed);
}

void AK_PlayerController::OnPawnDestroyed(AActor* DestroyedActor)
{
	TArray<AActor*> actorLists;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), actorLists);
	
	if (actorLists.Num() >0)
	{
		const FTransform spawnTransform = actorLists[0]->GetActorTransform();
		
		AK_PlayerCharacter* respawnCharacter = GetWorld()->SpawnActor<AK_PlayerCharacter>(CharacterClass, spawnTransform);
		ensureMsgf(respawnCharacter, TEXT("리스폰 가능한 캐릭터가 없음"));
		
		Possess(respawnCharacter);
	}
}

