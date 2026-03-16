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

bool UGA_ProjectileAttack::PrepareProjectileData(const URSSkillData* SkillData, TSubclassOf<ABaseProjectile>& OutClass, FRSSkillInitData& OutInitData)
{
	
    const FName SkillID = SkillData->SkillID;

    // GDS 조회 
    GET_GI_SUBSYSTEM_FROM(UGameDataSubsystem, GDS, GetWorld()->GetGameInstance());

    FSkillStaticData SkillStaticData;
    if (!GDS->GetSkillData(SkillID, SkillStaticData))
    {
        KHS_WARN(TEXT("SkillID 조회 실패: %s"),*SkillID.ToString());
        return false;
    }

    FSkillEffectData EffectData;
    if (!GDS->GetSkillEffectData(SkillStaticData.SkillEffectID, EffectData))
    {
       KHS_WARN(TEXT("SkillEffectID 조회 실패: %s"), *SkillStaticData.SkillEffectID.ToString() );
       return false;
    }
    
    // 투사체 클래스 로드 
    OutClass = Cast<UClass>(SkillStaticData.ProjectileClass.LoadSynchronous());

    if (!ensureMsgf(OutClass, TEXT("ProjectileClass 로드 실패: %s"), *SkillID.ToString()))
    {
        return false;
    }

    //데미지 GE 로드 - 실패시 리턴
    TSubclassOf<UGameplayEffect> DamageGEClass = Cast<UClass>(SkillStaticData.DamageGEClass.LoadSynchronous());

    if (!ensureMsgf(DamageGEClass, TEXT("DamageGE 로드 실패: %s"), *SkillID.ToString()))
    {
        return false;
    }
    
    //상태이상 GE 로드 - 실패시 스킵
    TSubclassOf<UGameplayEffect> StatusGEClass = nullptr;
    if (!SkillStaticData.StatusGEClass.IsNull())
    {
        StatusGEClass = Cast<UClass>(SkillStaticData.StatusGEClass.LoadSynchronous());
        if (!StatusGEClass)
        {
            KHS_WARN(TEXT("StatusGE 로드 실패 — 스킵: %s"), *SkillID.ToString());
        }
    }

    // InitData 채우기 
    OutInitData.SkillID        = SkillID;
    OutInitData.SkillEffectID  = SkillStaticData.SkillEffectID;
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
	
    return true; 
}