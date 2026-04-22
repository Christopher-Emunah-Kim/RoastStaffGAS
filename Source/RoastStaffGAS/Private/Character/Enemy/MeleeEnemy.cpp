// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Enemy/MeleeEnemy.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"
#include "GAS/Tags/RSGameplayTags.h"
#include "Components/SphereComponent.h"
#include "System/LoggingSystem.h"


AMeleeEnemy::AMeleeEnemy()
{
	// 공격 범위 판정용 SphereComponent 생성
	AttackRangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AttackRangeSphere"));
	AttackRangeSphere->SetupAttachment(RootComponent);
	AttackRangeSphere->SetCollisionProfileName(TEXT("Trigger")); //TODO : 콜리전 채널 만들어야함.(Projectile/Trigger)
	AttackRangeSphere->SetSphereRadius(100.f); //임시값. InitializeMeleeParams에서 갱신
}

void AMeleeEnemy::BeginPlay()
{
	Super::BeginPlay();
	//TODO [테스트] 임시
	//InitializeEnemy(TEXT("ENM_Melee_01"));
	
	AttackRangeSphere->OnComponentBeginOverlap.AddDynamic(this, &AMeleeEnemy::OnAttackRangeBeginOverlap);
	AttackRangeSphere->OnComponentEndOverlap.AddDynamic(this, &AMeleeEnemy::OnAttackRangeEndOverlap);
}

void AMeleeEnemy::OnAttackRangeBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor)
	{
		return;
	}

	// 플레이어 팀 태그 보유 여부 확인
	UAbilitySystemComponent* TargetASC = nullptr;
	if (IAbilitySystemInterface* ASInterface = Cast<IAbilitySystemInterface>(OtherActor))
	{
		TargetASC = ASInterface->GetAbilitySystemComponent();
	}

	if (!TargetASC)
	{
		return;
	}

	if (!TargetASC->HasMatchingGameplayTag(RSTags::Team_Player))
	{
		return;
	}

	// 플레이어가 이미 사망한 경우 무시
	if (TargetASC->HasMatchingGameplayTag(RSTags::State_Dead))
	{
		return;
	}

	//즉시 1회 공격 후 타이머로 반복 공격.(Overlap 유지동안)
	AttackTarget = OtherActor;
	TryAttack(OtherActor);
	
	GetWorld()->GetTimerManager().SetTimer(AttackRepeatTimerHandle, 
		FTimerDelegate::CreateUObject(this, &AMeleeEnemy::TryAttack, OtherActor),
		AttackCooldown,true );
	
}

void AMeleeEnemy::OnAttackRangeEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// 공격 중이던 대상이 범위를 벗어난 경우 타이머 정지
	if (OtherActor != AttackTarget.Get())
	{
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(AttackRepeatTimerHandle);
	AttackTarget.Reset();

	KHS_DEBUG(TEXT("%s — 플레이어 공격 범위 이탈. 공격 타이머 정지."), *GetName());
}

void AMeleeEnemy::TryAttack(AActor* Target)
{
	if (!AttackGEClass)
	{
		KHS_WARN(TEXT("%s — AttackGEClass Is NULL."), *GetName());
		return;
	}

	// 플레이어가 사망하여 사라지는 경우
	if (!IsValid(Target))
	{
		GetWorld()->GetTimerManager().ClearTimer(AttackRepeatTimerHandle);
		AttackTarget.Reset();
		return;
	}
	
	UAbilitySystemComponent* TargetASC = Cast<IAbilitySystemInterface>(Target)->GetAbilitySystemComponent();

	if (!TargetASC)
	{
		KHS_WARN(TEXT("%s — TargetASC is NULL."), *GetName());
		return;
	}

	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponent();
	if (!ensureMsgf(SourceASC, TEXT("%s — SourceASC is NULL."), *GetName()))
	{
		return;
	}

	// GE 스펙 생성 후 플레이어 ASC에 적용
	FGameplayEffectContextHandle ContextHandle = SourceASC->MakeEffectContext();
	ContextHandle.AddSourceObject(this);

	FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(AttackGEClass, 1.f, ContextHandle);
	if (!SpecHandle.IsValid())
	{
		KHS_WARN(TEXT("%s — GE Spec 생성 실패."), *GetName());
		
		return;
	}

	// SetByCaller로 실제 데미지 값 전달
	// TODO : (임시) AttackDamage 데이터가 없으므로 임시 값 사용
	AttackDamage = 5.f;
	SpecHandle.Data->SetByCallerTagMagnitudes.Add(RSTags::Data_EnemyAttackDamage, AttackDamage);
	TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

	KHS_DEBUG(TEXT("%s — 근접 공격. 대상: %s / 데미지: %.0f"), *GetName(), *Target->GetName(), AttackDamage);

}

void AMeleeEnemy::InitializeMeleeParams(float InAttackDamage, float InAttackCooldown, float InAttackRange)
{
	AttackDamage   = InAttackDamage;
	AttackCooldown = InAttackCooldown;
	// 공격 범위 콜라이더 반경
	if (AttackRangeSphere)
	{
		AttackRangeSphere->SetSphereRadius(InAttackRange);
	}

	KHS_DEBUG(TEXT("%s — MeleeParams 초기화. Damage: %.0f / Cooldown: %.1f / Range: %.0f"), *GetName(), AttackDamage, AttackCooldown, InAttackRange);
}
