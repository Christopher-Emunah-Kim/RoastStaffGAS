// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Enemy/EnemyBaseCharacter.h"
#include "RoastStaffGAS.h"
#include "AbilitySystemComponent.h"
#include "BrainComponent.h"
#include "Character/Enemy/EnemyAIController.h"
#include "GAS/Attributes/EnemyAttributeSet.h"
#include "GAS/Attributes/BaseAttributeSet.h"
#include "Subsystems/GameDataSubsystem.h"
#include "Subsystems/PoolingSubsystem.h"
#include "Subsystems/StageManagerSubsystem.h"
#include "Data/DataTableStructs.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/Tags/RSGameplayTags.h"
#include "Components/WidgetComponent.h"
#include "UI/Enemy/EnemyHPBarWidget.h"

AEnemyBaseCharacter::AEnemyBaseCharacter()
{
	// ASC
	ASC = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("ASC"));

	// EnemyAttributeSet - ASC 등록
	EnemyAttributeSet = CreateDefaultSubobject<UEnemyAttributeSet>(TEXT("EnemyAttributeSet"));
	ASC->AddAttributeSetSubobject<UEnemyAttributeSet>(EnemyAttributeSet);

	// HPBar WidgetComponent — 월드 스페이스, 머리 위 부착
	// 위치는 BP 에디터의 컴포넌트 트랜스폼에서 조정 가능
	HPBarWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("HPBarWidgetComp"));
	HPBarWidgetComp->SetupAttachment(RootComponent);
	HPBarWidgetComp->SetWidgetSpace(EWidgetSpace::Screen);  // 항상 카메라를 향함
	HPBarWidgetComp->SetRelativeLocation(FVector(0.f, 0.f, 90.f));
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
	
	// HPBar 바인딩 및 데미지 델리게이트 구독
	SetupHPBar();
	SetupDamageDelegate();

	bIsInitialized = true;

	KHS_INFO(TEXT("%s — 초기화 완료. EnemyID: %s / HP: %.0f / Speed: %.0f"), *GetName(), *EnemyID.ToString(), EnemyData.MaxHP, EnemyData.MoveSpeed);
}

void AEnemyBaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	// 풀에서 처음 생성 시 비활성 상태로 시작 (BaseSummonObject 동일 패턴)
	OnPoolDeactivate();
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
	// 공통 사망 프로세스 (bIsDead 가드, GA 종료, GE 제거, State.Dead 태그, 충돌 비활성화)
	Super::HandleDeath();

	// HPBar 숨김
	if (HPBarWidgetComp)
	{
		HPBarWidgetComp->SetVisibility(false);
	}

	// DamageDelegate 명시적 해제
	if (ASC)
	{
		ASC->GetGameplayAttributeValueChangeDelegate(UBaseAttributeSet::GetCurrentHPAttribute())
			.RemoveAll(this);
	}

	// 생존 목록에서 먼저 제거한 후 처치 이벤트 브로드캐스트
	if (UStageManagerSubsystem* StageMgr = GetWorld()->GetSubsystem<UStageManagerSubsystem>())
	{
		StageMgr->UnregisterAliveEnemy(this);
	}
	OnEnemyKilledDel.Broadcast(EnemyID);

	// 사망 연출 후 풀 반납 (SetLifeSpan 대신 ReturnToPool — 액터 재사용)
	GetWorldTimerManager().SetTimer(
		DeathReturnTimerHandle,
		[this]()
		{
			if (UPoolingSubsystem* PoolSys = GetWorld()->GetSubsystem<UPoolingSubsystem>())
			{
				PoolSys->ReturnToPool(this);
			}
		},
		DeathPoolReturnDelay,
		false
	);

	KHS_INFO(TEXT("%s — 에너미 사망 처리 완료. EnemyID: %s"), *GetName(), *EnemyID.ToString());
}

void AEnemyBaseCharacter::OnPoolActivate()
{
	// 가시성 및 충돌 활성화
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);

	// ASC Actor 정보 갱신 및 State.Dead 태그 제거
	if (ASC)
	{
		ASC->InitAbilityActorInfo(this, this);

		FGameplayTagContainer DeadTag;
		DeadTag.AddTag(RSTags::State_Dead);
		ASC->RemoveLooseGameplayTags(DeadTag);
	}

	// HPBar 표시
	if (HPBarWidgetComp)
	{
		HPBarWidgetComp->SetVisibility(true);
	}

	// 재초기화 허용 플래그 리셋
	bIsInitialized = false;
	bIsDead        = false;
}

void AEnemyBaseCharacter::OnPoolDeactivate()
{
	// 가시성 및 충돌 비활성화
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);

	// 진행 중인 Ability 강제 종료 (ASC가 준비된 경우만)
	if (ASC)
	{
		ASC->CancelAllAbilities();
	}

	// AI 중단
	if (AEnemyAIController* AIC = Cast<AEnemyAIController>(GetController()))
	{
		AIC->StopAI();
	}

	// 사망 복귀 타이머 정리 (비정상 반납 시 중복 실행 방지)
	GetWorldTimerManager().ClearTimer(DeathReturnTimerHandle);
}


void AEnemyBaseCharacter::SetupHPBar()
{
	if (!HPBarWidgetClass)
	{
		KHS_WARN(TEXT("%s — HPBarWidgetClass가 할당되지 않았습니다. HPBar를 건너뜁니다."), *GetName());
		return;
	}

	HPBarWidgetComp->SetWidgetClass(HPBarWidgetClass);

	UEnemyHPBarWidget* HPBarWidget = Cast<UEnemyHPBarWidget>(HPBarWidgetComp->GetWidget());
	if (!HPBarWidget)
	{
		KHS_WARN(TEXT("%s — HPBarWidget 획득 실패. WidgetClass 타입을 확인하세요."), *GetName());
		return;
	}

	HPBarWidget->BindToASC(ASC);
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