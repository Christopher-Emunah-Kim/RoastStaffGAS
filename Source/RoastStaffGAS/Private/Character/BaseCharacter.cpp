// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/BaseCharacter.h"
#include "AbilitySystemComponent.h"
#include "GAS/Attributes/BaseAttributeSet.h"
#include "GAS/Tags/RSGameplayTags.h"
#include "Character/Player/RSPlayerController.h"
#include "System/LoggingSystem.h"

ABaseCharacter::ABaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void ABaseCharacter::HandleDeath()
{
	// 중복 사망 방지
	if (bIsDead)
	{
		return;
	}
	bIsDead = true;

	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC)
	{
		KHS_WARN(TEXT(" %s : ASC IS NULL"), *GetName());
		return;
	}

	//모든 GA 강제 종료
	ASC->CancelAllAbilities();

	//모든 Infinite GE 제거
	ASC->RemoveActiveEffects(FGameplayEffectQuery());
	
	//사망 상태 태그 부여
	FGameplayTagContainer DeadTag;
	DeadTag.AddTag(RSTags::State_Dead);
	ASC->AddLooseGameplayTags(DeadTag);

	//충돌 비활성화 — 사망 후 투사체가 시체에 맞지 않도록
	SetActorEnableCollision(false);

	// 사망 애니메이션 재생 로직

	KHS_DEBUG(TEXT(" %s — 부모 공통 사망 처리 완료."), *GetName());

	// 고유 사망 처리는 자식 클래스에서 오버라이드
}

void ABaseCharacter::SetupDamageDelegate()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC)
	{
		KHS_WARN(TEXT("%s — SetupDamageDelegate: ASC is null"), *GetName());
		return;
	}

	ASC->GetGameplayAttributeValueChangeDelegate(UBaseAttributeSet::GetCurrentHPAttribute())
		.AddUObject(this, &ABaseCharacter::OnCurrentHPChangedForDamage);
}

void ABaseCharacter::OnCurrentHPChangedForDamage(const FOnAttributeChangeData& Data)
{
	const float Damage = Data.OldValue - Data.NewValue;
	if (Damage <= 0.f)
	{
		return;
	}

	ARSPlayerController* PC = Cast<ARSPlayerController>(GetWorld()->GetFirstPlayerController());
	if (!PC)
	{
		KHS_WARN(TEXT("%s — OnCurrentHPChangedForDamage: ARSPlayerController 획득 실패"), *GetName());
		return;
	}

	const FVector WorldPos = GetActorLocation() + FVector(0.f, 0.f, DamageWidgetZOffset);
	PC->SpawnFloatingDamage(WorldPos, Damage);

	if (IsPlayerControlled())
	{
		PC->FlashHUDDamageIndicator();
	}
}

void ABaseCharacter::BindAttributeDelegates()
{
	UBaseAttributeSet* AS = GetBaseAttributeSet();

	if (!ensureMsgf(AS, TEXT("BindAttributeDelegates FAILED — AS IS NULL")))
	{
		return;
	}

	// 사망 델리게이트 구독 (재초기화 시 중복 방지)
	AS->OnDeathDel.RemoveDynamic(this, &ABaseCharacter::HandleDeath);
	AS->OnDeathDel.AddDynamic(this, &ABaseCharacter::HandleDeath);

	KHS_DEBUG(TEXT(" %s — ATTRIBUTE DELEGATE BINDING SUCCESS"), *GetName());
}

void ABaseCharacter::ApplyPassiveEffects(const TArray<TSoftClassPtr<UGameplayEffect>>& DefaultEffects)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();

	if (!ensureMsgf(ASC, TEXT("ApplyPassiveEffects FAILED — ASC IS NULL")))
	{
		return;
	}

	for (const TSoftClassPtr<UGameplayEffect>& SoftGEClass : DefaultEffects)
	{
		if (SoftGEClass.IsNull())
		{
			KHS_WARN(TEXT("%s — 패시브 GE 경로가 비어있습니다. 패시브 GE 적용 스킵."), *GetName());
			continue;
		}

		TSubclassOf<UGameplayEffect> GEClass = SoftGEClass.LoadSynchronous();
		if (!GEClass)
		{
			// 로드 실패 시 — 해당 GE만 스킵, 나머지 계속 진행
			KHS_WARN(TEXT(" %s — 패시브 GE 로드 실패. 패시브 GE 적용 스킵: %s"),*GetName(), *SoftGEClass.ToString());
			continue;
		}

		FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
		ContextHandle.AddSourceObject(this);

		FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(GEClass, 1.f, ContextHandle);
		if (SpecHandle.IsValid())
		{
			ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
		else
		{
			KHS_WARN(TEXT(" %s — GE Spec 생성 실패. 스킵: %s"), *GetName(), *SoftGEClass.ToString());
		}
	}
}
