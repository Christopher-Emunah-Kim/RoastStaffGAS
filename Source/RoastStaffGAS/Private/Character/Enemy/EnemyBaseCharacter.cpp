// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Enemy/EnemyBaseCharacter.h"
#include "RoastStaffGAS.h"
#include "AbilitySystemComponent.h"
#include "Character/Enemy/EnemyAIController.h"
#include "GAS/Attributes/EnemyAttributeSet.h"
#include "Subsystems/GameDataSubsystem.h"
#include "Data/DataTableStructs.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/Tags/RSGameplayTags.h"

AEnemyBaseCharacter::AEnemyBaseCharacter()
{
	// ASC
	ASC = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("ASC"));

	// EnemyAttributeSet - ASC 등록
	EnemyAttributeSet = CreateDefaultSubobject<UEnemyAttributeSet>(TEXT("EnemyAttributeSet"));
	ASC->AddAttributeSetSubobject<UEnemyAttributeSet>(EnemyAttributeSet);
}


bool AEnemyBaseCharacter::StartEnemyAI(FEnemyStaticData EnemyData)
{
	AEnemyAIController* EnemyAIC = Cast<AEnemyAIController>(GetController());
	if (!ensureMsgf(EnemyAIC, TEXT("%s — CANNOT FIND EnemyAIController."), *GetName()))
	{
		return false;
	}

	UBehaviorTree* BTAsset = EnemyData.BehaviorTree.LoadSynchronous();
	if (!BTAsset)
	{
		KHS_WARN(TEXT("%s — BehaviorTree 로드 실패. EnemyID: %s"), *GetName(), *EnemyID.ToString());
		check(false);
		return false;
	}

	EnemyAIC->StartAI(BTAsset);
	return true;
}

void AEnemyBaseCharacter::InitializeEnemy(FName InEnemyID)
{
	if (bIsInitialized)
	{
		KHS_WARN(TEXT("%s — 중복 초기화 시도 무시. EnemyID: %s"), *GetName(), *InEnemyID.ToString());
		return;
	}

	// ASC 초기화
	EnemyID = InEnemyID;
	InitializeAbilitySystem();

	// 스탯 주입
	FEnemyStaticData EnemyData;
	if (!ApplyStatData(EnemyData))
	{
		return;
	}

	// AI BT 시작
	if (!StartEnemyAI(EnemyData))
	{
		return;
	}
	
	bIsInitialized = true;

	KHS_INFO(TEXT("%s — 초기화 완료. EnemyID: %s / HP: %.0f / Speed: %.0f"), *GetName(), *EnemyID.ToString(), EnemyData.MaxHP, EnemyData.MoveSpeed);
}

void AEnemyBaseCharacter::BeginPlay()
{
	Super::BeginPlay();
}

UAbilitySystemComponent* AEnemyBaseCharacter::GetAbilitySystemComponent() const
{
	return ASC;
}

UBaseAttributeSet* AEnemyBaseCharacter::GetBaseAttributeSet() const
{
	return EnemyAttributeSet;
}

void AEnemyBaseCharacter::InitializeAbilitySystem()
{
	if (!ensureMsgf(ASC, TEXT("%s — ASC IS NULL."), *GetName()))
	{
		return;
	}

	ASC->InitAbilityActorInfo(this, this);

	// 델리게이트 바인딩 (BaseCharacter 공통)
	BindAttributeDelegates();
	
	// 에너미 팀 태그 부여
	FGameplayTagContainer EnemyTag;
	EnemyTag.AddTag(RSTags::Team_Enemy);
	ASC->AddLooseGameplayTags(EnemyTag);

	KHS_INFO(TEXT("%s — ASC 초기화 완료"), *GetName());
}

void AEnemyBaseCharacter::HandleDeath()
{
	// 공통 사망 프로세스 (GA 종료, GE 제거, State.Dead 태그, 충돌 비활성화)
	Super::HandleDeath();

	// StageWaveSubsystem에 처치 통보
	OnEnemyKilledDel.Broadcast(EnemyID);

	// 사망 연출 후 액터 소멸 (2초)
	SetLifeSpan(2.f);

	KHS_INFO(TEXT("%s — 에너미 사망 처리 완료. EnemyID: %s"), *GetName(), *EnemyID.ToString());
}


bool AEnemyBaseCharacter::ApplyStatData(FEnemyStaticData& EnemyData)
{
	//  GDS에서 DT_Enemy 조회
	GET_GI_SUBSYSTEM(UGameDataSubsystem, GDS);

	if (!GDS->GetEnemyData(EnemyID, EnemyData))
	{
		// 조회 실패 시 크래시
		KHS_WARN(TEXT("%s — DT_Enemy 조회 실패. EnemyID: %s"), *GetName(), *EnemyID.ToString());
		check(false);
		return false;
	}

	// 기본 스탯 주입
	ASC->SetNumericAttributeBase(EnemyAttributeSet->GetMaxHPAttribute(),    EnemyData.MaxHP);
	ASC->SetNumericAttributeBase(EnemyAttributeSet->GetCurrentHPAttribute(),EnemyData.MaxHP);
	ASC->SetNumericAttributeBase(EnemyAttributeSet->GetMoveSpeedAttribute(),EnemyData.MoveSpeed);

	// 이동 속도 반영
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (!MoveComp)
	{
		KHS_WARN(TEXT("Invalid MoveComp"));
		return false;
	}
	MoveComp->MaxWalkSpeed = EnemyData.MoveSpeed;
	return true;
}