// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Enemy/BossEnemy.h"
#include "Character/Enemy/EnemyAIController.h"
#include "GAS/Attributes/BaseAttributeSet.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GAS/Tags/RSGameplayTags.h"
#include "RoastStaffGAS.h"
#include "Objects/Projectile/EnemyProjectile.h"

ABossEnemy::ABossEnemy()
{
}

// ─────────────────────────────────────────────────────────────────────────────
// 초기화
// ─────────────────────────────────────────────────────────────────────────────

void ABossEnemy::BeginPlay()
{
	Super::BeginPlay();

	// HP 변화 구독 — 페이즈 전환 감시
	if (UAbilitySystemComponent* MyASC = GetAbilitySystemComponent())
	{
		MyASC->GetGameplayAttributeValueChangeDelegate(UBaseAttributeSet::GetCurrentHPAttribute())
			.AddUObject(this, &ABossEnemy::OnHPChanged);
	}
}

void ABossEnemy::InitializeBossParams(float InAttackDamage, const FEnemyExtData& ExtData)
{
	AttackDamage      = InAttackDamage;
	PreferredRange    = ExtData.PreferredRange;
	MaxAttackRange    = ExtData.MaxAttackRange;
	ProjectileSpeed   = ExtData.ProjectileSpeed;
	ProjectileLifetime = ExtData.ProjectileLifetime;

	ShockwaveRadius      = ExtData.ShockwaveRadius;
	ShockwaveDamage      = ExtData.ShockwaveDamage;
	ShockwaveCooldown    = ExtData.ShockwaveCooldown;
	ShockwavePrepareTime = ExtData.ShockwavePrepareTime;

	Phase2HPRatio       = ExtData.Phase2HPRatio;
	Phase2MoveSpeedMult = ExtData.Phase2MoveSpeedMult;
	Phase2DamageMult    = ExtData.Phase2DamageMult;

	// 전환 연출 에셋 동기 로드 (InitializeEnemy 직후 1회)
	if (!ExtData.Phase2TransitionFX.IsNull())
	{
		LoadedTransitionFX = ExtData.Phase2TransitionFX.LoadSynchronous();
	}
	if (!ExtData.Phase2TransitionMontage.IsNull())
	{
		LoadedTransitionMontage = ExtData.Phase2TransitionMontage.LoadSynchronous();
	}

	KHS_INFO(TEXT("%s — BossParams 초기화. ATK: %.0f / Phase2HPRatio: %.2f / ShockwaveR: %.0f"),
		*GetName(), AttackDamage, Phase2HPRatio, ShockwaveRadius);
}

// ─────────────────────────────────────────────────────────────────────────────
// 페이즈 전환
// ─────────────────────────────────────────────────────────────────────────────

void ABossEnemy::OnHPChanged(const FOnAttributeChangeData& Data)
{
	if (UBaseAttributeSet* AS = GetBaseAttributeSet())
	{
		CheckPhaseTransition(Data.NewValue, AS->GetMaxHP());
	}
}

void ABossEnemy::CheckPhaseTransition(float NewHP, float MaxHP)
{
	if (bPhaseTransitioned || MaxHP <= 0.f)
	{
		return;
	}

	if ((NewHP / MaxHP) <= Phase2HPRatio)
	{
		StartPhaseTransition();
	}
}

void ABossEnemy::StartPhaseTransition()
{
	// 중복 트리거 방지 — 타이머 설정 전 즉시 플래그 세팅
	bPhaseTransitioned = true;

	KHS_INFO(TEXT("%s — Phase2 전환 시작."), *GetName());

	// AI 일시 중단
	if (AEnemyAIController* AIC = Cast<AEnemyAIController>(GetController()))
	{
		AIC->PauseAI();
	}

	// 전환 연출
	if (LoadedTransitionFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(), LoadedTransitionFX, GetActorLocation());
	}

	float TransitionDuration = DefaultTransitionDuration;
	if (LoadedTransitionMontage)
	{
		PlayAnimMontage(LoadedTransitionMontage);
		TransitionDuration = LoadedTransitionMontage->GetPlayLength();
	}

	GetWorldTimerManager().SetTimer(PhaseTransitionTimerHandle,this, &ABossEnemy::ActivatePhase2,
		TransitionDuration, false);
}

void ABossEnemy::ActivatePhase2()
{
	// 이동 속도 부스트
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed *= Phase2MoveSpeedMult;
	}

	// BB 키 설정 + AI 재개
	AEnemyAIController* AIC = Cast<AEnemyAIController>(GetController());
	if (!ensureMsgf(AIC, TEXT("AIC CAST FAILED")))
	{
		return;
	}
	AIC->ResumeAI();
	
	UBlackboardComponent* BB = AIC->GetBlackboardComponent();
	if (!ensureMsgf(BB, TEXT("Get BlackBoard is FAILED")))
	{
		return;	
	}
	BB->SetValueAsBool(AEnemyAIController::BBKey_bIsPhase2, true);
	
	KHS_INFO(TEXT("%s — Phase2 활성. MoveSpeedMult: %.2f / DamageMult: %.2f"),*GetName(), Phase2MoveSpeedMult, Phase2DamageMult);
}

// ─────────────────────────────────────────────────────────────────────────────
// Shockwave
// ─────────────────────────────────────────────────────────────────────────────

bool ABossEnemy::IsShockwaveReady() const
{
	return GetWorld() && (GetWorld()->GetTimeSeconds() - LastShockwaveTime >= ShockwaveCooldown);
}

void ABossEnemy::MarkShockwaveUsed()
{
	if (GetWorld())
	{
		LastShockwaveTime = GetWorld()->GetTimeSeconds();
	}
}

void ABossEnemy::ExecuteShockwave()
{
	if (!ShockwaveGEClass)
	{
		KHS_WARN(TEXT("%s — ShockwaveGEClass 미할당."), *GetName());
		return;
	}

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC || !PC->GetPawn())
	{
		return;
	}

	AActor* Player = PC->GetPawn();
	const float DistSq = FVector::DistSquared(GetActorLocation(), Player->GetActorLocation());
	if (DistSq > FMath::Square(ShockwaveRadius))
	{
		KHS_DEBUG(TEXT("%s — Shockwave: 플레이어 범위 밖. 스킵."), *GetName());
		return;
	}

	ApplyShockwaveDamage(Player);
}

void ABossEnemy::ApplyShockwaveDamage(AActor* Target)
{
	IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(Target);
	if (!ASCInterface)
	{
		return;
	}

	UAbilitySystemComponent* TargetASC = ASCInterface->GetAbilitySystemComponent();
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponent();
	if (!TargetASC)
	{
		return;
	}

	UAbilitySystemComponent* EffectSource = SourceASC ? SourceASC : TargetASC;
	FGameplayEffectContextHandle Context  = EffectSource->MakeEffectContext();
	FGameplayEffectSpecHandle    Spec     = EffectSource->MakeOutgoingSpec(ShockwaveGEClass, 1.f, Context);
	if (!Spec.IsValid())
	{
		return;
	}

	Spec.Data->SetByCallerTagMagnitudes.Add(RSTags::Data_EnemyAttackDamage, ShockwaveDamage);
	TargetASC->ApplyGameplayEffectSpecToSelf(*Spec.Data);

	KHS_DEBUG(TEXT("%s — Shockwave 데미지 적용. 데미지: %.0f"), *GetName(), ShockwaveDamage);
}

// ─────────────────────────────────────────────────────────────────────────────
// Phase2 방사형 투사체
// ─────────────────────────────────────────────────────────────────────────────

void ABossEnemy::FireSpreadProjectile()
{
	const float DamageWithMult = AttackDamage * Phase2DamageMult;

	for (int32 i = 0; i < 8; ++i)
	{
		const float AngleRad = FMath::DegreesToRadians(i * 45.f);
		const FVector Direction(FMath::Cos(AngleRad), FMath::Sin(AngleRad), 0.f);
		LaunchEnemyProjectile(Direction, DamageWithMult);
	}

	KHS_DEBUG(TEXT("%s — 보스 방사형 투사체 발사. 데미지: %.0f"), *GetName(), DamageWithMult);
}

// ─────────────────────────────────────────────────────────────────────────────
// 사망
// ─────────────────────────────────────────────────────────────────────────────

void ABossEnemy::HandleDeath()
{
	// 페이즈 전환 타이머 진행 중 사망 시 즉시 취소
	GetWorldTimerManager().ClearTimer(PhaseTransitionTimerHandle);
	OnBossKilledDel.Broadcast();

	Super::HandleDeath();
}
