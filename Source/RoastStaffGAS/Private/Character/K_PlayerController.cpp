// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/K_PlayerController.h"
#include "Character/K_PlayerCharacter.h"
#include "Character/K_PlayerState.h"
#include "System/K_LoggingSystem.h"
#include "System/K_UIManagerSubsystem.h"
#include "UI/K_HUDWidget.h"

#include "EnhancedInputSubsystems.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"

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
		return;
	}
	
	UEnhancedInputLocalPlayerSubsystem* subsys = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	check(subsys);
	subsys->AddMappingContext(IMC, 0);
	
}

void AK_PlayerController::OnPossess(APawn* InPawn)
{

	Super::OnPossess(InPawn);
	
	InPawn->OnDestroyed.AddDynamic(this, &AK_PlayerController::OnPawnDestroyed);
	
	//UI초기화를 약간 지연시켜야 PlayerState실행이 보장됨
	//네트워크에 따라 PlayerState 리플리케이션이 늦어질수있음
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &AK_PlayerController::InitializePersistentUI);
}

void AK_PlayerController::InitializePersistentUI()
{
	auto* UIManager = GetGameInstance()->GetSubsystem<UK_UIManagerSubsystem>();
	check(UIManager);
	
	if (!ensureMsgf(HUDWidgetClass, TEXT("[PlayerController] Invalid HUD Widget class")))
	{
		return;
	}
	
	//HUD Widget 생성 및 하위 위젯 ASC 바인딩
	UIManager->OpenUI<UK_HUDWidget>(HUDWidgetClass);
	UK_HUDWidget* HUDUI = UIManager->GetOrCreateWidget<UK_HUDWidget>(HUDWidgetClass);
	
	//PlayerState에서 ASC가져오기
	AK_PlayerState* ps = GetPlayerState<AK_PlayerState>();
	check(ps);
	
	UAbilitySystemComponent* ASC = ps->GetAbilitySystemComponent();
	check(ASC);
	
	HUDUI->BindToASC(ASC);
	KHS_INFO(TEXT("[PlayerController] HUDWidget created and bound to PlayerState ASC"));
}

void AK_PlayerController::HandleUICloseRequest(class UK_BaseWidget* RequestingWidget)
{
	if (!ensureMsgf(RequestingWidget, TEXT("[PlayerController] Invalid RequestingWidget")))
	{
		return;
	}
	
	auto* UIManager = GetGameInstance()->GetSubsystem<UK_UIManagerSubsystem>();
	check(UIManager);
	
	UIManager->CloseUI(RequestingWidget);
	RequestingWidget->OnCloseUIRequested.RemoveDynamic(this, &AK_PlayerController::HandleUICloseRequest);
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

