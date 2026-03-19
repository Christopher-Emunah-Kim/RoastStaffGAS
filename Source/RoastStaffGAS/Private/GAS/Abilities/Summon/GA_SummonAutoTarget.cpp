// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/Summon/GA_SummonAutoTarget.h"
#include "RoastStaffGAS.h"
#include "Character/BaseCharacter.h"
#include "GAS/Tags/RSGameplayTags.h"
#include "Objects/Data/RSSkillData.h"
#include "Subsystems/EquipmentSubsystem.h"

void UGA_SummonAutoTarget::OnAbilityActivated(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	const URSSkillData* SkillData = Cast<URSSkillData>(GetCurrentSourceObject());                                
      if (!ensureMsgf(SkillData, TEXT("SourceObject가 URSSkillData가 아님")))                                      
      {                                                                                                            
          EndAbility(Handle, ActorInfo, ActivationInfo, true, true);                                               
          return;                                                                                                  
      }                                                                                                          
      CachedSkillID = SkillData->SkillID;                                                                          
                                                                                                                   
      if (!LoadSummonData(CachedExecData, CachedSummonParam))                                                      
      {                                                                                                            
          EndAbility(Handle, ActorInfo, ActivationInfo, true, true);                                               
          return;                                                                                                  
      }                                                                                                            
                                                                                                                   
      // Active 모드: 클릭 대기                                                                                    
      if (CheckIsActiveSlot())                                                                                   
      {                                                                                                            
          UAbilityTask_WaitConfirmCancel* WaitTask = UAbilityTask_WaitConfirmCancel::WaitConfirmCancel(this);      
          WaitTask->OnConfirm.AddDynamic(this, &UGA_SummonAutoTarget::OnConfirm);                                  
          WaitTask->OnCancel.AddDynamic(this, &UGA_SummonAutoTarget::OnCancel);                                    
          WaitTask->ReadyForActivation();                                                                          
          return;                                                                                                  
      }                                                                                                            
                                                                                                                 
      // 자동 모드: 즉시 소환                                                                                      
      const FVector SummonLocation = DetermineSummonLocation();                                                  
      if (SummonLocation.IsZero())                                                                                 
      {                                                                                                          
          EndAbility(Handle, ActorInfo, ActivationInfo, true, false);                                              
          return;                                                                                                  
      }
	
      SpawnSummonObject(SummonLocation);                                                                           
      EndAbility(Handle, ActorInfo, ActivationInfo, true, false);       
}

FVector UGA_SummonAutoTarget::DetermineSummonLocation()
{
	AActor* NearestEnemy = nullptr;
	float   NearestDistSq = FLT_MAX;
	
	const float SearchRange = FindNearestEnemy(NearestEnemy, NearestDistSq);

	if (!NearestEnemy)
	{
		KHS_INFO(TEXT("SearchRange(%.0f) 내 적 없음. SkillID: %s"),	SearchRange, *CachedSkillID.ToString());
		return CachedInstigator->GetActorLocation() + CachedInstigator->GetActorForwardVector() * 1000.f;  
	}

	KHS_INFO(TEXT("FIND ENEMY. SKillID : %s, 적과의 거리 : %.0f"), *CachedSkillID.ToString(), NearestDistSq);
	return NearestEnemy->GetActorLocation();

}

const float UGA_SummonAutoTarget::FindNearestEnemy(AActor*& NearestEnemy, float& NearestDistSq)
{
	const float SearchRange = CachedSummonParam.SearchRange;
	
	const FVector Origin = GetOwningActorFromActorInfo()->GetActorLocation();
	TArray<FOverlapResult> Overlaps;
	const FCollisionShape Sphere = FCollisionShape::MakeSphere(SearchRange);
	GetWorld()->OverlapMultiByChannel(Overlaps, Origin, FQuat::Identity, ECC_Pawn, Sphere);
	
	//SearchRange내에 적대태그 가진 유효 적이 있을때 해당 위치 반환. 없으면 기본값 반환.
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
			NearestEnemy  = Candidate;
		}
	}
	return SearchRange;
}

void UGA_SummonAutoTarget::OnConfirm()
{
	const FVector SummonLocation = DetermineSummonLocation();                                                    
	if (!SummonLocation.IsZero())                                                                              
	{
		SpawnSummonObject(SummonLocation);
	}                                                                                                            
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);

}

void UGA_SummonAutoTarget::OnCancel()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);  
}

bool UGA_SummonAutoTarget::CheckIsActiveSlot() const
{
	GET_GI_SUBSYSTEM_FROM(UEquipmentSubsystem, EQS, GetWorld()->GetGameInstance());                              
                                                                                                                   
	for (int32 i = 0; ; ++i)                                                                                     
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
