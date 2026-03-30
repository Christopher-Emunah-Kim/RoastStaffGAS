// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/GA_ProjectileAttack.h"
#include "RoastStaffGAS.h"
#include "GAS/Tags/RSGameplayTags.h"
#include "Objects/Data/RSSkillData.h"
#include "Subsystems/GameDataSubsystem.h"
#include "Objects/Projectile/BaseProjectile.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Character/BaseCharacter.h"
#include "Data/EnumTypes.h"

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



bool UGA_ProjectileAttack::PrepareProjectileData(const URSSkillData* SkillData, TSubclassOf<ABaseProjectile>& OutClass, FProjectileInitData& OutInitData)
{
	GET_GI_SUBSYSTEM_FROM(UGameDataSubsystem, GDS, GetWorld()->GetGameInstance());
	FSkillExecutionData ExecData;
	if (!GDS->GetSkillExecutionData(SkillData->SkillID, ExecData, SkillData->WeaponID))
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

	if (!HandleExtraParametersByType(OutInitData, ExecData))
	{
		return false;
	}

	return true;
}


void UGA_ProjectileAttack::BuildInitData(const FSkillExecutionData& ExecData, TSubclassOf<UGameplayEffect> DamageGEClass, TSubclassOf<UGameplayEffect> StatusGEClass, FProjectileInitData& OutInitData) const
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
	OutInitData.MoveType      = ExecData.MoveType;
	OutInitData.HitType       = ExecData.HitType;
}


bool UGA_ProjectileAttack::HandleExtraParametersByType(FProjectileInitData& OutInitData, const FSkillExecutionData& ExecData)
{
	//타입별 추가 데이터 채우기
	switch (ExecData.MoveType)
	{
	case EMoveType::HOMING: // HOMING 타입 추가 처리
		{
			if (!HandleHomingType(OutInitData, ExecData))
			{
				return false;
			}
		}
		break;
	case EMoveType::ARC: // ARC 타입 추가 처리
		{
			if (!HandleArcType(OutInitData, ExecData))
			{
				return false;
			}
		}
		break;
		
	default:
		{
			//파라미터 추가 작업이므로 별도 처리 필요없음.
		}
	}
	
	switch (ExecData.HitType)
	{
	case EHitType::AREA: // AREA 타입 추가 처리
		{
			if (!HandleAreaType(OutInitData, ExecData))
			{
				return false;
			}
		}
		break;
	case EHitType::PIERCE: // PIERCE 타입 추가 처리
		{
			if (!HandlePierceType(OutInitData, ExecData))
			{
				return false;
			}
		}
		break;
		
	default:
		{
			//파라미터 추가 작업이므로 별도 처리 필요없음.
		}
	}
	
	return true;
}

bool UGA_ProjectileAttack::HandleHomingType(FProjectileInitData& OutInitData, const FSkillExecutionData& ExecData)
{
	GET_GI_SUBSYSTEM_FROM(UGameDataSubsystem, GDS, GetWorld()->GetGameInstance());  
	FSkillAttackMoveTypeParamsHoming HomingData;
	if (!GDS->GetMoveTypeData(ExecData.SkillEffectID, HomingData))
	{
		KHS_WARN(TEXT("Homing 파라미터 조회 실패. SkillEffectID: %s"), *ExecData.SkillEffectID.ToString());
		return false;
	}
		
	OutInitData.TurnSpeed = HomingData.TurnSpeed;

	// LockRange 내 최근접 Enemy 탐색
	AActor* NearestEnemy = FindNearestEnemy(HomingData);

	if (!NearestEnemy)
	{
		KHS_WARN(TEXT("LockRange 내 적 없음 — 직선 비행 폴백. SkillEffectID: %s"), *ExecData.SkillEffectID.ToString());
		// HomingTarget은 null 유지 → OnProjectileInitialized에서 직선 비행 폴백
		return true;
	}
		
	OutInitData.HomingTarget = NearestEnemy->GetRootComponent();
	return true;
}



AActor* UGA_ProjectileAttack::FindNearestEnemy(FSkillAttackMoveTypeParamsHoming HomingData)
{
	AActor* NearestEnemy = nullptr;
	const FVector CasterLocation = CachedInstigator->GetActorLocation();
	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(CachedInstigator.Get());

	GetWorld()->OverlapMultiByChannel(Overlaps,	CasterLocation,	FQuat::Identity,ECC_Pawn,
									  FCollisionShape::MakeSphere(HomingData.LockRange),	QueryParams	);

	float MinDistSq = MAX_FLT;

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* OverlappedActor = Overlap.GetActor();
		if (!OverlappedActor)
		{
			continue;
		}

		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OverlappedActor);
		if (!TargetASC || !TargetASC->HasMatchingGameplayTag(RSTags::Team_Enemy))
		{
			continue;
		}

		const float DistSq = FVector::DistSquared(CasterLocation, OverlappedActor->GetActorLocation());
		if (DistSq < MinDistSq)
		{
			MinDistSq = DistSq;
			NearestEnemy = OverlappedActor;
		}
	}
	return NearestEnemy;
}


bool UGA_ProjectileAttack::HandleArcType(FProjectileInitData& OutInitData, const FSkillExecutionData& ExecData)
{
	GET_GI_SUBSYSTEM_FROM(UGameDataSubsystem, GDS, GetWorld()->GetGameInstance());  
	FSkillAttackMoveTypeParamsArc ArcData;
	if (!GDS->GetMoveTypeData(ExecData.SkillEffectID, ArcData))
	{
		KHS_WARN(TEXT("Arc 파라미터 조회 실패. SkillEffectID: %s"), *ExecData.SkillEffectID.ToString());
		return false;
	}
	
	constexpr int32 LAUNCH_ANGLE_CLAMP = 80.f;
	OutInitData.LaunchAngle  = FMath::Clamp(ArcData.LaunchAngle, -LAUNCH_ANGLE_CLAMP, LAUNCH_ANGLE_CLAMP);
	OutInitData.GravityScale = ArcData.GravityScale;
	return true;
}


bool UGA_ProjectileAttack::HandleAreaType(FProjectileInitData& OutInitData, const FSkillExecutionData& ExecData)
{
	GET_GI_SUBSYSTEM_FROM(UGameDataSubsystem, GDS, GetWorld()->GetGameInstance());
	FSkillAttackHitTypeParamsArea AreaData;
	if (!GDS->GetHitTypeData(ExecData.SkillEffectID, AreaData))
	{
		KHS_WARN(TEXT("Area 파라미터 조회 실패. SkillEffectID: %s"), *ExecData.SkillEffectID.ToString());
		return false;
	}
	OutInitData.HitRadius = AreaData.HitRadius;
	return true;
}


bool UGA_ProjectileAttack::HandlePierceType(FProjectileInitData& OutInitData, const FSkillExecutionData& ExecData)
{
	GET_GI_SUBSYSTEM_FROM(UGameDataSubsystem, GDS, GetWorld()->GetGameInstance());
	FSkillAttackHitTypeParamsPierce PierceData;
	
	if (!GDS->GetHitTypeData(ExecData.SkillEffectID, PierceData))
	{
		KHS_WARN(TEXT("Pierce 파라미터 조회 실패. SkillEffectID: %s"), *ExecData.SkillEffectID.ToString());
		return false;
	}
	
	OutInitData.PierceCount = PierceData.PierceCount;
	OutInitData.DamageDecay = PierceData.DamageDecay;
	
	return true;
}