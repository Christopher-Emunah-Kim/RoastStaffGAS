// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/GA_ProjectileAttack.h"
#include "RoastStaffGAS.h"
#include "GAS/Tags/RSGameplayTags.h"
#include "Objects/Data/RSSkillData.h"
#include "Subsystems/GameDataSubsystem.h"
#include "Objects/Projectile/BaseProjectile.h"

UGA_ProjectileAttack::UGA_ProjectileAttack()
{
	// Event_Weapon_Fire 태그로 트리거되도록 등록
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag    = RSTags::Event_Weapon_Fire_Projectile;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);
}

void UGA_ProjectileAttack::OnAbilityActivated(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	// SourceObject에서 SkillID 추출
    const URSSkillData* SkillData = Cast<URSSkillData>(GetCurrentSourceObject());

    if (!ensureMsgf(SkillData, TEXT("SourceObject가 URSSkillData가 아님")))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    TSubclassOf<ABaseProjectile> ProjectileClass;
    FProjectileInitData InitData;
    
    if (!PrepareProjectileData(SkillData, ProjectileClass, InitData))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }
    
    //투사체 스폰
    SpawnProjectiles(ProjectileClass, InitData);

    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UGA_ProjectileAttack::BuildInitData(const FSkillExecutionData& ExecData,
	TSubclassOf<UGameplayEffect> DamageGEClass, TSubclassOf<UGameplayEffect> StatusGEClass,
	FProjectileInitData& OutInitData) const
{
	// InitData 채우기 
	OutInitData.SkillID       = ExecData.SkillID;                                                              
	OutInitData.SkillEffectID = ExecData.SkillEffectID;                                                          
	OutInitData.DamageGEClass = DamageGEClass;
	OutInitData.StatusGEClass = StatusGEClass;                                                                   
	OutInitData.InstigatorASC = GetOwnerASC();                                                                   
	OutInitData.Amount        = ExecData.Amount;                                                                 
	OutInitData.Speed         = ExecData.Speed;                                                                  
	OutInitData.Lifetime      = ExecData.Lifetime;                                                               
	OutInitData.SpawnPattern  = ExecData.SpawnPattern;                                                           
	OutInitData.SpawnCount    = ExecData.SpawnCount;                                                             
	OutInitData.SpreadAngle   = ExecData.SpreadAngle;
}


bool UGA_ProjectileAttack::PrepareProjectileData(const URSSkillData* SkillData, TSubclassOf<ABaseProjectile>& OutClass, FProjectileInitData& OutInitData)
{
	GET_GI_SUBSYSTEM_FROM(UGameDataSubsystem, GDS, GetWorld()->GetGameInstance());  
	FSkillExecutionData ExecData;
	if (!GDS->GetSkillExecutionData(SkillData->SkillID, ExecData))                                               
	{                                                                                                          
		KHS_WARN(TEXT("GetSkillExecutionData 실패. SkillID: %s"), *SkillData->SkillID.ToString());
		return false;
	}
	if (!LoadRequiredClass(ExecData.ProjectileClass, OutClass, ExecData.SkillID))                                
	{
		return false;                                                                                            
	}                                                                                                          
                                                                                                                   
	TSubclassOf<UGameplayEffect> DamageGEClass;                                                                  
	if (!LoadRequiredClass(ExecData.DamageGEClass, DamageGEClass, ExecData.SkillID))                             
	{                                                                                                            
		return false;                                                                                          
	}                                                                                                            
   
	TSubclassOf<UGameplayEffect> StatusGEClass = LoadOptionalClass(ExecData.StatusGEClass, ExecData.SkillID);    
                                                                                                                 
	BuildInitData(ExecData, DamageGEClass, StatusGEClass, OutInitData);                                          
	return true;                 
}
