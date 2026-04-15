// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Ingame/CharacterStatPopupWidget.h"
#include "RoastStaffGAS.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "AbilitySystemComponent.h"
#include "GAS/Attributes/BaseAttributeSet.h"
#include "GAS/Attributes/PlayerAttributeSet.h"
#include "Character/Player/RSPlayerState.h"

void UCharacterStatPopupWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (Btn_Close)
	{
		Btn_Close->OnClicked.AddDynamic(this, &UCharacterStatPopupWidget::OnCloseClicked);
	}
}

void UCharacterStatPopupWidget::OpenUI()
{
	Super::OpenUI();

	ARSPlayerState* PS = GetOwningPlayer() ? Cast<ARSPlayerState>(GetOwningPlayer()->PlayerState) : nullptr;
	if (!PS)
	{
		KHS_WARN(TEXT("OpenUI — PlayerState 없음"));
		return;
	}

	UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
	UPlayerAttributeSet* PlayerAS = PS->GetPlayerAttributeSet();
	if (!ASC || !PlayerAS)
	{
		KHS_WARN(TEXT("OpenUI — ASC 또는 PlayerAttributeSet 없음"));
		return;
	}

	CachedASC      = ASC;
	CachedPlayerAS = PlayerAS;

	// ASC 어트리뷰트 델리게이트 구독 — GE 타입·변경 경로 무관하게 자동 호출됨
	ASC->GetGameplayAttributeValueChangeDelegate(UPlayerAttributeSet::GetATKAttribute())           .AddUObject(this, &UCharacterStatPopupWidget::OnStatChanged);
	ASC->GetGameplayAttributeValueChangeDelegate(UPlayerAttributeSet::GetDEFAttribute())           .AddUObject(this, &UCharacterStatPopupWidget::OnStatChanged);
	ASC->GetGameplayAttributeValueChangeDelegate(UPlayerAttributeSet::GetAttackSpeedAttribute())   .AddUObject(this, &UCharacterStatPopupWidget::OnStatChanged);
	ASC->GetGameplayAttributeValueChangeDelegate(UPlayerAttributeSet::GetCriticalRateAttribute())  .AddUObject(this, &UCharacterStatPopupWidget::OnStatChanged);
	ASC->GetGameplayAttributeValueChangeDelegate(UPlayerAttributeSet::GetCriticalDamageAttribute()).AddUObject(this, &UCharacterStatPopupWidget::OnStatChanged);
	ASC->GetGameplayAttributeValueChangeDelegate(UBaseAttributeSet::GetMoveSpeedAttribute())       .AddUObject(this, &UCharacterStatPopupWidget::OnStatChanged);
	ASC->GetGameplayAttributeValueChangeDelegate(UBaseAttributeSet::GetCurrentHPAttribute())       .AddUObject(this, &UCharacterStatPopupWidget::OnStatChanged);
	ASC->GetGameplayAttributeValueChangeDelegate(UBaseAttributeSet::GetMaxHPAttribute())           .AddUObject(this, &UCharacterStatPopupWidget::OnStatChanged);

	RefreshAllStats();
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void UCharacterStatPopupWidget::CloseUI()
{
	if (CachedASC.IsValid())
	{
		UAbilitySystemComponent* ASC = CachedASC.Get();
		ASC->GetGameplayAttributeValueChangeDelegate(UPlayerAttributeSet::GetATKAttribute())           .RemoveAll(this);
		ASC->GetGameplayAttributeValueChangeDelegate(UPlayerAttributeSet::GetDEFAttribute())           .RemoveAll(this);
		ASC->GetGameplayAttributeValueChangeDelegate(UPlayerAttributeSet::GetAttackSpeedAttribute())   .RemoveAll(this);
		ASC->GetGameplayAttributeValueChangeDelegate(UPlayerAttributeSet::GetCriticalRateAttribute())  .RemoveAll(this);
		ASC->GetGameplayAttributeValueChangeDelegate(UPlayerAttributeSet::GetCriticalDamageAttribute()).RemoveAll(this);
		ASC->GetGameplayAttributeValueChangeDelegate(UBaseAttributeSet::GetMoveSpeedAttribute())       .RemoveAll(this);
		ASC->GetGameplayAttributeValueChangeDelegate(UBaseAttributeSet::GetCurrentHPAttribute())       .RemoveAll(this);
		ASC->GetGameplayAttributeValueChangeDelegate(UBaseAttributeSet::GetMaxHPAttribute())           .RemoveAll(this);
	}

	CachedASC.Reset();
	CachedPlayerAS.Reset();

	SetVisibility(ESlateVisibility::Collapsed);
	Super::CloseUI();
}

void UCharacterStatPopupWidget::OnCloseClicked()
{
	CloseUI();
}

void UCharacterStatPopupWidget::OnStatChanged(const FOnAttributeChangeData& Data)
{
	RefreshAllStats();
}

void UCharacterStatPopupWidget::RefreshAllStats()
{
	if (!CachedPlayerAS.IsValid() || !CachedASC.IsValid())
	{
		return;
	}

	UPlayerAttributeSet* PA  = CachedPlayerAS.Get();
	UAbilitySystemComponent* ASC = CachedASC.Get();

	if (Txt_ATK)
	{
		Txt_ATK->SetText(MakeStatText(
			ASC->GetNumericAttributeBase(UPlayerAttributeSet::GetATKAttribute()), PA->GetATK()));
	}
	if (Txt_DEF)
	{
		Txt_DEF->SetText(MakeStatText(
			ASC->GetNumericAttributeBase(UPlayerAttributeSet::GetDEFAttribute()), PA->GetDEF()));
	}
	if (Txt_AttackSpeed)
	{
		Txt_AttackSpeed->SetText(MakeStatText(
			ASC->GetNumericAttributeBase(UPlayerAttributeSet::GetAttackSpeedAttribute()), PA->GetAttackSpeed()));
	}
	if (Txt_MoveSpeed)
	{
		Txt_MoveSpeed->SetText(MakeStatText(
			ASC->GetNumericAttributeBase(UBaseAttributeSet::GetMoveSpeedAttribute()), PA->GetMoveSpeed()));
	}
	if (Txt_CritRate)
	{
		Txt_CritRate->SetText(MakePercentText(
			ASC->GetNumericAttributeBase(UPlayerAttributeSet::GetCriticalRateAttribute()), PA->GetCriticalRate()));
	}
	if (Txt_CritDmg)
	{
		// CritDmg는 패시브 미적용 — 단순 표시
		Txt_CritDmg->SetText(FText::AsNumber(PA->GetCriticalDamage()));
	}
	if (Txt_HP)
	{
		// HP: "현재HP / 최대HP (base MaxHP)" — MaxHP 보너스 표시
		const float MaxBase = ASC->GetNumericAttributeBase(UBaseAttributeSet::GetMaxHPAttribute());
		const float MaxAggr = PA->GetMaxHP();
		const float MaxBonus = MaxAggr - MaxBase;
		if (FMath::IsNearlyZero(MaxBonus, 0.5f))
		{
			Txt_HP->SetText(FText::Format(INVTEXT("{0} / {1}"),
				FMath::RoundToInt(PA->GetCurrentHP()), FMath::RoundToInt(MaxAggr)));
		}
		else
		{
			Txt_HP->SetText(FText::Format(INVTEXT("{0} / {1} (+{2})"),
				FMath::RoundToInt(PA->GetCurrentHP()), FMath::RoundToInt(MaxAggr),
				FMath::RoundToInt(MaxBonus)));
		}
	}
}

FText UCharacterStatPopupWidget::MakeStatText(float Base, float Aggregated) const
{
	const int32 BaseInt  = FMath::RoundToInt(Base);
	const float Bonus    = Aggregated - Base;
	const int32 BonusInt = FMath::RoundToInt(Bonus);

	if (BonusInt == 0)
	{
		return FText::AsNumber(BaseInt);
	}
	if (BonusInt > 0)
	{
		return FText::Format(INVTEXT("{0} (+{1})"), BaseInt, BonusInt);
	}
	return FText::Format(INVTEXT("{0} ({1})"), BaseInt, BonusInt); // 디버프: "-X" 그대로 출력
}

FText UCharacterStatPopupWidget::MakePercentText(float Base, float Aggregated) const
{
	const float Bonus    = Aggregated - Base;
	const int32 BonusPct = FMath::RoundToInt(Bonus * 100.f);

	if (BonusPct == 0)
	{
		return FText::AsPercent(Base);
	}
	if (BonusPct > 0)
	{
		return FText::Format(INVTEXT("{0} (+{1}%)"), FText::AsPercent(Base), BonusPct);
	}
	return FText::Format(INVTEXT("{0} ({1}%)"), FText::AsPercent(Base), BonusPct);
}
