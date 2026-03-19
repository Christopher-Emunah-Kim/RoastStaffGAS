// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/GA_SummonBase.h"

#include "RoastStaffGAS.h"
#include "GAS/Tags/RSGameplayTags.h"
#include "Objects/Data/RSSkillData.h"
#include "Objects/Summon/BaseSummonObject.h"
#include "Subsystems/EquipmentSubsystem.h"
#include "Subsystems/GameDataSubsystem.h"

UGA_SummonBase::UGA_SummonBase()
{
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag    = RSTags::Event_Weapon_Fire_Summon;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);

}

void UGA_SummonBase::OnAbilityActivated(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	const URSSkillData* SkillData = Cast<URSSkillData>(GetCurrentSourceObject());
	if (!ensureMsgf(SkillData, TEXT("SourceObject가 URSSkillData가 아님")))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	CachedSkillID = SkillData->SkillID;
	
	//GDS에서 필요 데이터 로드
	if (!LoadSummonData(CachedExecData, CachedSummonParam))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	const FVector SummonLocation = DetermineSummonLocation();
	if (SummonLocation.IsZero())
	{
		KHS_INFO(TEXT("소환 위치 없음 (적 없음). SkillID: %s"), *CachedSkillID.ToString());
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}
	
	//위치에 오브젝트 소환
	SpawnSummonObject(SummonLocation);
	
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UGA_SummonBase::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	KHS_INFO(TEXT("EndAbility 진입. CachedSkillID: %s"), *CachedSkillID.ToString());
	if (!CachedSkillID.IsNone())
	{
		GET_GI_SUBSYSTEM_FROM(UEquipmentSubsystem, EQS, GetWorld()->GetGameInstance());
		EQS->OnSummonAbilityEnded(CachedSkillID);
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_SummonBase::SpawnSummonObject(const FVector& Location)
{
	FSummonObjectInitData InitData;
	FActorSpawnParameters SpawnParams;
	TSubclassOf<AActor> SummonClass;
	
	//로드한 데이터 세팅
	if (!SetSummonData(InitData, SpawnParams, SummonClass))
	{
		KHS_WARN(TEXT("SummonData Setting FAILED. SkillID: %s"), *CachedSkillID.ToString());
		return;
	}
	
	//SpawnActorDeferred BeginPlay 호출 지연 -> FinishSpawning시점에 BeginPlay호출해서 InitData 넘기기.
	ABaseSummonObject* SummonObject = GetWorld()->SpawnActorDeferred<ABaseSummonObject>(SummonClass, FTransform(Location), SpawnParams.Owner, SpawnParams.Instigator);
	if (!SummonObject)
	{
		KHS_WARN(TEXT("SummonObject Spawn FAILED. SkillID: %s"), *CachedSkillID.ToString());
		return;
	}

	SummonObject->InitSummon(InitData);
	SummonObject->FinishSpawning(FTransform(Location));
}


bool UGA_SummonBase::SetSummonData(FSummonObjectInitData& InitData, FActorSpawnParameters& SpawnParams, TSubclassOf<AActor>& SummonClass)
{
	//필요 클래스/데이터 로드
	if (!LoadRequiredClass(CachedExecData.SummonObjectClass, SummonClass, CachedSkillID))
	{
		return false;
	}
	
	TSubclassOf<UGameplayEffect> DamageGEClass, StatusGEClass = LoadOptionalClass(CachedExecData.StatusGEClass, CachedSkillID);
	if (!LoadRequiredClass(CachedExecData.DamageGEClass, DamageGEClass, CachedSkillID))
	{
		return false;
	}

	InitData.SkillID		= CachedSkillID;
	InitData.DamageGEClass	= DamageGEClass;
	InitData.StatusGEClass	= StatusGEClass;
	InitData.InstigatorASC	= GetOwnerASC();
	InitData.Amount			= CachedExecData.Amount;
	InitData.Lifetime		= CachedExecData.Lifetime;
	InitData.SummonRadius	= CachedSummonParam.SummonRadius;
	
	SpawnParams.Owner		= GetOwningActorFromActorInfo();
	SpawnParams.Instigator	= Cast<APawn>(GetOwningActorFromActorInfo());
	
	return true;
}

bool UGA_SummonBase::LoadSummonData(FSkillExecutionData& OutExecData,
	FSkillAttackMoveTypeParamsSummon& OutSummonParam) const
{
	GET_GI_SUBSYSTEM_FROM(UGameDataSubsystem, GDS, GetWorld()->GetGameInstance());

	if (!GDS->GetSkillExecutionData(CachedSkillID, OutExecData))
	{
		KHS_WARN(TEXT("GetSkillExecutionData 실패. SkillID: %s"), *CachedSkillID.ToString());
		return false;
	}

	if (!GDS->GetMoveTypeData<FSkillAttackMoveTypeParamsSummon>(OutExecData.SkillEffectID,OutSummonParam))
	{
		KHS_WARN(TEXT("Summon MoveTypeData 없음. SkillEffectID: %s"), *OutExecData.SkillEffectID.ToString());
		return false;
	}
	return true;
}
