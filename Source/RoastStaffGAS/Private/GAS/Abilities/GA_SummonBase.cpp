// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/GA_SummonBase.h"

#include "RoastStaffGAS.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Character/Player/RSPlayerController.h"
#include "GAS/Tags/RSGameplayTags.h"
#include "Objects/Data/RSSkillData.h"
#include "Objects/Projectile/BaseProjectile.h"
#include "Objects/Summon/BaseSummonObject.h"
#include "Objects/Summon/SummonPreviewObject.h"
#include "Subsystems/EquipmentSubsystem.h"
#include "Subsystems/GameDataSubsystem.h"
#include "Subsystems/PoolingSubsystem.h"

UGA_SummonBase::UGA_SummonBase()
{
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag    = RSTags::Event_Weapon_Fire_Summon;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);
	
	AbilityTags.AddTag(RSTags::Ability_Skill_Summon);
	ActivationBlockedTags.AddTag(RSTags::State_Dead);  
}

void UGA_SummonBase::OnAbilityActivated(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	if (!LoadSkillData())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	if (CheckIsActiveSlot())
	{
		HandleActiveMode(); //자식마다 다름
		return;
	}
	
	const FVector SummonLocation = DetermineSummonLocation();
	if (SummonLocation.IsZero())
	{
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
	if (CachedPreviewObject.Get())
	{
		CachedPreviewObject->Destroy();
		CachedPreviewObject = nullptr;
	}
	
	if (!CachedSkillID.IsNone())
	{
		GET_GI_SUBSYSTEM_FROM(UEquipmentSubsystem, EQS, GetWorld()->GetGameInstance());
		EQS->OnSummonAbilityEnded(CachedSkillID);
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

FVector UGA_SummonBase::DetermineSummonLocation()
{
	// 액티브(수동) 모드: 마우스 에임 좌표에 소환
	if (CheckIsActiveSlot())
	{
		ARSPlayerController* PC = Cast<ARSPlayerController>(GetWorld()->GetFirstPlayerController());
		check(PC);

		const FVector AimLoc = PC->GetCachedAimLocation();
		if (AimLoc.IsZero())
		{
			KHS_WARN(TEXT("CachedAimLocation 미초기화 (커서 화면 밖). SkillID: %s"), *CachedSkillID.ToString());
		}
		return AimLoc;
	}

	// 자동 모드: SearchRange 내 최근접 적 위치에 소환
	AActor* NearestEnemy = nullptr;
	FindNearestEnemy(NearestEnemy);
	if (!NearestEnemy)
	{
		KHS_INFO(TEXT("SearchRange(%.0f) 내 적 없음 — 발동 생략. SkillID: %s"), CachedSummonParam.SearchRange, *CachedSkillID.ToString());
		return FVector::ZeroVector;
	}
	return NearestEnemy->GetActorLocation();
}


bool UGA_SummonBase::LoadSkillData()
{
	const URSSkillData* SkillData = Cast<URSSkillData>(GetCurrentSourceObject());
	if (!ensureMsgf(SkillData, TEXT("SourceObject가 URSSkillData가 아님")))
	{
		return false;
	}
	
	CachedSkillID = SkillData->SkillID;
	
	//GDS에서 필요 소환 데이터 로드
	if (!LoadSummonData(CachedExecData, CachedSummonParam))
	{
		return false;
	}
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

void UGA_SummonBase::HandleActiveMode()
{
	SpawnPreviewObject();

	//GAS 어빌리티 WaitTask에 등록.
	UAbilityTask_WaitConfirmCancel* WaitTask = UAbilityTask_WaitConfirmCancel::WaitConfirmCancel(this);          
	WaitTask->OnConfirm.AddDynamic(this, &UGA_SummonBase::OnConfirm);                                  
	WaitTask->OnCancel.AddDynamic(this, &UGA_SummonBase::OnCancel);                                              
	WaitTask->ReadyForActivation();  
    KHS_INFO(TEXT("WaitConfirmCancel 등록 완료.")); 
    //EndAbility는 OnConfirm / OnCancel에서 호출
}

void UGA_SummonBase::SpawnSummonObject(const FVector& Location)
{
	FSummonObjectInitData InitData;                                                                              
    TSubclassOf<AActor> SummonClass;                                                                             
                                                                                                                 
    if (!SetSummonData(InitData, SummonClass))                                                                 
    {                                                                                                            
        KHS_WARN(TEXT("SummonData 세팅 실패. SkillID: %s"), *CachedSkillID.ToString());
        return;                                                                                                  
    }                                                                                                            
     
	GET_WORLD_SUBSYSTEM(UPoolingSubsystem, PoolSys);                                                                                                        
                                                                                                                 
    ABaseSummonObject* SummonObject = Cast<ABaseSummonObject>(PoolSys->SpawnPooledActor(SummonClass, FTransform(Location)));                                                                      
                                                                                                               
    if (!SummonObject)                                                                                           
    {                                                                                                            
        KHS_WARN(TEXT("SummonObject SpawnPooled 실패. SkillID: %s"), *CachedSkillID.ToString());                 
        return;                                                                                                  
    }                                                                                                            
     
    SummonObject->SetOwner(GetOwningActorFromActorInfo());                                                       
    SummonObject->SetInstigator(Cast<APawn>(GetOwningActorFromActorInfo()));                                     
    SummonObject->InitSummon(InitData);   
}


bool UGA_SummonBase::SetSummonData(FSummonObjectInitData& InitData, TSubclassOf<AActor>& SummonClass)
{
	//필요 클래스/데이터 로드
	if (!LoadRequiredClass(CachedExecData.SummonObjectClass, SummonClass, CachedSkillID))
	{
		return false;
	}
	TSubclassOf<UGameplayEffect> DamageGEClass;
	if (!LoadRequiredClass(CachedExecData.DamageGEClass, DamageGEClass, CachedSkillID))
	{
		return false;
	}
	TSubclassOf<UGameplayEffect> StatusGEClass = LoadOptionalClass(CachedExecData.StatusGEClass, CachedSkillID);

	InitData.SkillID		= CachedSkillID;
	InitData.DamageGEClass	= DamageGEClass;
	InitData.StatusGEClass	= StatusGEClass;
	InitData.InstigatorASC	= GetOwnerASC();
	InitData.Amount			= CachedExecData.Amount;
	InitData.Lifetime		= CachedExecData.Lifetime;
	InitData.SummonRadius	= CachedSummonParam.SummonRadius;
	
	return true;
}

void UGA_SummonBase::SpawnPreviewObject()
{
	TSubclassOf<AActor> PreviewClass;
	if (!LoadRequiredClass(CachedExecData.SummonPreviewClass, PreviewClass, CachedSkillID))
	{
		KHS_WARN(TEXT("SummonPreviewClass 로드 실패. SkillID: %s"), *CachedSkillID.ToString());
		return;
	}
	
	ARSPlayerController* PC = Cast<ARSPlayerController>(GetWorld()->GetFirstPlayerController());
	const FVector InitLocation = PC ? PC->GetCachedAimLocation() : FVector::ZeroVector;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwningActorFromActorInfo();
	CachedPreviewObject = GetWorld()->SpawnActor<ASummonPreviewObject>(PreviewClass, InitLocation,FRotator::ZeroRotator, SpawnParams);
}

bool UGA_SummonBase::CheckIsActiveSlot() const
{
	GET_GI_SUBSYSTEM_FROM(UEquipmentSubsystem, EQS, GetWorld()->GetGameInstance());                              
                                                                                                                   
	for (int32 i = 0; i < EQS->GetSlotCount(); ++i)                                                                                     
	{                                                                                                            
		const FWeaponSlotInstanceData* Slot = EQS->GetSlotData(i);                                               
		if (!Slot)                                                                                               
		{
			break;                                                                                               
		}                                                                                                      
		if (Slot->SlotEquipData.SkillID == CachedSkillID)
		{                                                                                                        
			return Slot->bIsActive;                                                                              
		}                                                                                                        
	}                                                                                                            
	return false; 
}

void UGA_SummonBase::FindNearestEnemy(AActor*& OutEnemy) const
{
	const float SearchRange = CachedSummonParam.SearchRange;
	const FVector Origin = GetOwningActorFromActorInfo()->GetActorLocation();

	TArray<FOverlapResult> Overlaps;
	const FCollisionShape Sphere = FCollisionShape::MakeSphere(SearchRange);
	GetWorld()->OverlapMultiByChannel(Overlaps, Origin, FQuat::Identity, ECC_Pawn, Sphere);

	float NearestDistSq = FLT_MAX;
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Candidate = Overlap.GetActor();
		if (!Candidate)
		{
			continue;
		}

		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Candidate);
		if (!TargetASC || !TargetASC->HasMatchingGameplayTag(RSTags::Team_Enemy))
		{
			continue;
		}

		const float DistSq = FVector::DistSquared(Origin, Candidate->GetActorLocation());
		if (DistSq < NearestDistSq)
		{
			NearestDistSq = DistSq;
			OutEnemy      = Candidate;
		}
	}
}

void UGA_SummonBase::OnConfirm()
{
	const FVector SummonLocation = DetermineSummonLocation();  
	KHS_INFO(TEXT("OnConfirm 수신. 소환 위치: %s"), *SummonLocation.ToString()); 
	if (!SummonLocation.IsZero())                                                                              
	{
		SpawnSummonObject(SummonLocation);
	}                                                                                                            
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_SummonBase::OnCancel()
{
	KHS_INFO(TEXT("소환 취소됨. SkillID: %s"),  *CachedSkillID.ToString());
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

