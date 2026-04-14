// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Ingame/CharacterStatPopupWidget.h"
#include "RoastStaffGAS.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
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

	UPlayerAttributeSet* PlayerAS = PS->GetPlayerAttributeSet();
	if (!PlayerAS)
	{
		KHS_WARN(TEXT("OpenUI — PlayerAttributeSet 없음"));
		return;
	}

	CachedPlayerAS = PlayerAS;
	CachedBaseAS   = PlayerAS; // UPlayerAttributeSet은 UBaseAttributeSet 상속

	PlayerAS->OnPlayerStatChangedDel.AddUniqueDynamic(this, &UCharacterStatPopupWidget::OnPlayerStatChanged);
	PlayerAS->OnMoveSpeedChangedDel.AddUniqueDynamic(this, &UCharacterStatPopupWidget::OnMoveSpeedChanged);
	PlayerAS->OnHealthChangedDel.AddUniqueDynamic(this, &UCharacterStatPopupWidget::OnHealthChanged);

	RefreshAllStats();
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void UCharacterStatPopupWidget::CloseUI()
{
	if (CachedPlayerAS.IsValid())
	{
		CachedPlayerAS->OnPlayerStatChangedDel.RemoveDynamic(this, &UCharacterStatPopupWidget::OnPlayerStatChanged);
		CachedPlayerAS->OnMoveSpeedChangedDel.RemoveDynamic(this, &UCharacterStatPopupWidget::OnMoveSpeedChanged);
		CachedPlayerAS->OnHealthChangedDel.RemoveDynamic(this, &UCharacterStatPopupWidget::OnHealthChanged);
	}

	CachedPlayerAS.Reset();
	CachedBaseAS.Reset();

	SetVisibility(ESlateVisibility::Collapsed);
	Super::CloseUI();
}

void UCharacterStatPopupWidget::OnCloseClicked()
{
	CloseUI();
}

void UCharacterStatPopupWidget::OnPlayerStatChanged(float NewATK, float NewDEF, float NewAttackSpeed, float NewCritRate, float NewCritDmg)
{
	if (Txt_ATK)        Txt_ATK->SetText(FText::AsNumber(FMath::RoundToInt(NewATK)));
	if (Txt_DEF)        Txt_DEF->SetText(FText::AsNumber(FMath::RoundToInt(NewDEF)));
	if (Txt_AttackSpeed) Txt_AttackSpeed->SetText(FText::AsNumber(FMath::RoundToInt(NewAttackSpeed)));
	if (Txt_CritRate)   Txt_CritRate->SetText(FText::AsPercent(NewCritRate));
	if (Txt_CritDmg)    Txt_CritDmg->SetText(FText::AsNumber(NewCritDmg));
}

void UCharacterStatPopupWidget::OnMoveSpeedChanged(float NewValue)
{
	if (Txt_MoveSpeed) Txt_MoveSpeed->SetText(FText::AsNumber(FMath::RoundToInt(NewValue)));
}

void UCharacterStatPopupWidget::OnHealthChanged(float NewHP, float NewMaxHP)
{
	if (Txt_HP) Txt_HP->SetText(FText::Format(INVTEXT("{0} / {1}"),	FMath::RoundToInt(NewHP), FMath::RoundToInt(NewMaxHP)));
}

void UCharacterStatPopupWidget::RefreshAllStats()
{
	if (!CachedPlayerAS.IsValid()) return;

	UPlayerAttributeSet* PA = CachedPlayerAS.Get();

	if (Txt_ATK)        Txt_ATK->SetText(FText::AsNumber(FMath::RoundToInt(PA->GetATK())));
	if (Txt_DEF)        Txt_DEF->SetText(FText::AsNumber(FMath::RoundToInt(PA->GetDEF())));
	if (Txt_AttackSpeed) Txt_AttackSpeed->SetText(FText::AsNumber(FMath::RoundToInt(PA->GetAttackSpeed())));
	if (Txt_CritRate)   Txt_CritRate->SetText(FText::AsPercent(PA->GetCriticalRate()));
	if (Txt_CritDmg)    Txt_CritDmg->SetText(FText::AsNumber(PA->GetCriticalDamage()));
	if (Txt_MoveSpeed)  Txt_MoveSpeed->SetText(FText::AsNumber(FMath::RoundToInt(PA->GetMoveSpeed())));
	if (Txt_HP)         Txt_HP->SetText(FText::Format(INVTEXT("{0} / {1}"),	FMath::RoundToInt(PA->GetCurrentHP()), FMath::RoundToInt(PA->GetMaxHP())));
}
