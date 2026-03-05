// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/GA_ProjectileAttack.h"
#include "GAS/Tags/RSGameplayTags.h"
#include "Objects/Data/RSSkillData.h"
#include "Subsystems/GameDataSubsystem.h"
#include "AbilitySystemComponent.h"
#include "Objects/Projectile/BaseProjectile.h"
#include "System/LoggingSystem.h"

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

    const FName SkillID = SkillData->SkillID;

    // GDS 조회 
    UGameInstance* GI = GetWorld()->GetGameInstance();
    check(GI);
    UGameDataSubsystem* GDS = GI->GetSubsystem<UGameDataSubsystem>();
    check(GDS);

    FSkillStaticData SkillStaticData;
    if (!GDS->GetSkillData(SkillID, SkillStaticData))
    {
        KHS_WARN(TEXT("SkillID 조회 실패: %s"),*SkillID.ToString());
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    FSkillEffectData EffectData;
    if (!GDS->GetSkillEffectData(SkillStaticData.SkillEffectID, EffectData))
    {
       KHS_WARN(TEXT("SkillEffectID 조회 실패: %s"), *SkillStaticData.SkillEffectID.ToString() );
       EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
       return;
    }
    
    // 투사체 클래스 로드 
    TSubclassOf<ABaseProjectile> ProjectileClass = Cast<UClass>(SkillStaticData.ProjectileClass.LoadSynchronous());

    if (!ensureMsgf(ProjectileClass, TEXT("ProjectileClass 로드 실패: %s"), *SkillID.ToString()))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    //데미지 GE 로드 - 실패시 리턴
    TSubclassOf<UGameplayEffect> DamageGEClass = Cast<UClass>(SkillStaticData.DamageGEClass.LoadSynchronous());

    if (!ensureMsgf(DamageGEClass, TEXT("DamageGE 로드 실패: %s"), *SkillID.ToString()))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
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
    FRSSkillInitData InitData;
    InitData.SkillID        = SkillID;
    InitData.SkillEffectID  = SkillStaticData.SkillEffectID;
    InitData.DamageGEClass  = DamageGEClass;
    InitData.StatusGEClass  = StatusGEClass;
    InitData.InstigatorASC  = GetOwnerASC();
    InitData.Damage         = EffectData.Damage;
    InitData.Speed          = EffectData.Speed;
    InitData.Lifetime       = EffectData.Lifetime;

    // SpawnData는 GDS에서 별도 조회 
    // 일단SINGLE 모드만 사용
    InitData.SpawnPattern    = ESpawnPattern::SINGLE;
    InitData.ProjectileCount = 1;
    InitData.SpreadAngle     = 0.f;

    //투사체 스폰
    SpawnProjectiles(ProjectileClass, InitData);

    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
