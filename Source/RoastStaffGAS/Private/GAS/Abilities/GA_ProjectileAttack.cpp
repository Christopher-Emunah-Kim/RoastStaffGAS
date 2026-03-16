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
    FRSSkillInitData InitData;
    
    if (!PrepareProjectileData(SkillData, ProjectileClass, InitData))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }
    
    //투사체 스폰
    SpawnProjectiles(ProjectileClass, InitData);

    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

bool UGA_ProjectileAttack::LoadSkillData(const FName SkillID, FSkillStaticData& OutSkillData, FSkillEffectData& OutEffectData) const
{
	GET_GI_SUBSYSTEM_FROM(UGameDataSubsystem, GDS, GetWorld()->GetGameInstance());

	if (!GDS->GetSkillData(SkillID, OutSkillData))
	{
		KHS_WARN(TEXT("SkillID 조회 실패: %s"),*SkillID.ToString());
		return false;
	}

	if (!GDS->GetSkillEffectData(OutSkillData.SkillEffectID, OutEffectData))
	{
		KHS_WARN(TEXT("SkillEffectID 조회 실패: %s"), *OutSkillData.SkillEffectID.ToString() );
		return false;
	}
	
	return true;
}

void UGA_ProjectileAttack::BuildInitData(const FSkillStaticData& SkillData, const FSkillEffectData& EffectData,
	TSubclassOf<UGameplayEffect> DamageGEClass, TSubclassOf<UGameplayEffect> StatusGEClass,	FRSSkillInitData& OutInitData) const
{
	// InitData 채우기 
	OutInitData.SkillID        = SkillData.SkillID;
	OutInitData.SkillEffectID  = SkillData.SkillEffectID;
	OutInitData.DamageGEClass  = DamageGEClass;
	OutInitData.StatusGEClass  = StatusGEClass;
	OutInitData.InstigatorASC  = GetOwnerASC();
	OutInitData.Damage         = EffectData.Damage;
	OutInitData.Speed          = EffectData.Speed;
	OutInitData.Lifetime       = EffectData.Lifetime;

	// TODO : SpawnData는 GDS에서 별도 조회 
	// 일단SINGLE 모드만 사용
	OutInitData.SpawnPattern    = ESpawnPattern::SINGLE;
	OutInitData.ProjectileCount = 1;
	OutInitData.SpreadAngle     = 0.f;	
}

bool UGA_ProjectileAttack::PrepareProjectileData(const URSSkillData* SkillData, TSubclassOf<ABaseProjectile>& OutClass, FRSSkillInitData& OutInitData)
{
    const FName SkillID = SkillData->SkillID;

    // GDS 조회 
    FSkillStaticData SkillStaticData;
    FSkillEffectData EffectData;
    if (!LoadSkillData(SkillID, SkillStaticData, EffectData))
    {
	    return false;
    }

    // 투사체/데미지GE/상태이상 GE 로드 
    if (!LoadRequiredClass(SkillStaticData.ProjectileClass, OutClass, SkillID))
    {
        return false;
    }
    TSubclassOf<UGameplayEffect> DamageGEClass;
    if (!LoadRequiredClass(SkillStaticData.DamageGEClass, DamageGEClass, SkillID))
    {
        return false;
    }
    TSubclassOf<UGameplayEffect> StatusGEClass = LoadOptionalClass(SkillStaticData.StatusGEClass, SkillID);
	
	//InitData 구성
	BuildInitData(SkillStaticData, EffectData, DamageGEClass, StatusGEClass, OutInitData);
	
    return true; 
}
