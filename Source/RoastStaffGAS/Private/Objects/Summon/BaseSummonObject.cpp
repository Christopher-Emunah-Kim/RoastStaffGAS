// Fill out your copyright notice in the Description page of Project Settings.


#include "Objects/Summon/BaseSummonObject.h"
#include "RoastStaffGAS.h"
#include "Engine/OverlapResult.h"
#include "GAS/Tags/RSGameplayTags.h"

// Sets default values
ABaseSummonObject::ABaseSummonObject()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

void ABaseSummonObject::InitSummon(const FSummonObjectInitData& InInitData)
{
	InitData = InInitData;
	bInitialized = true;
}

// Called when the game starts or when spawned
void ABaseSummonObject::BeginPlay()
{
	Super::BeginPlay();
	
	if (!bInitialized)                                                                                           
	{                                                                                                            
		KHS_WARN(TEXT("InitSummon 미호출. Actor: %s"), *GetName());                               
	}    
	
	ApplyGameplayEffectToArea();
	
	const float EffectLifeTime = InitData.Lifetime > 0.f? InitData.Lifetime : 1.f;
	SetLifeSpan(EffectLifeTime);
}

void ABaseSummonObject::ApplyGameplayEffectToArea()
{
	if (!InitData.DamageGEClass)
	{                                                                                                            
          KHS_WARN(TEXT("DamageGEClass null. SkillID: %s"), *InitData.SkillID.ToString());
          return;                                                                                                  
	}                                                                                                          
                                                                                                                   
	if (!InitData.InstigatorASC)                                                                               
	{
          KHS_WARN(TEXT("InstigatorASC null. SkillID: %s"), *InitData.SkillID.ToString());
          return;                                                                                                  
	}
                                                                                                                   
    TArray<FOverlapResult> Overlaps;                                                                             
    const FCollisionShape Sphere = FCollisionShape::MakeSphere(InitData.SummonRadius);                           
    GetWorld()->OverlapMultiByChannel(Overlaps, GetActorLocation(), FQuat::Identity, ECC_Pawn, Sphere);          
                   
	//GAS 액터들에게 데미지 GE 적용
	TSet<AActor*> HitActors;
    for (const FOverlapResult& Overlap : Overlaps)                                                               
    {                                                                                                            
        AActor* TargetActor = Overlap.GetActor();                                                                
        if (!TargetActor || HitActors.Contains(TargetActor))                                                                                      
        {
            continue;
        }                        
    	HitActors.Add(TargetActor); //중복 피하기
 
        UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);                                        
        if (!TargetASC)                                                                                          
        {                                                                                                      
            continue;
        }                                                                                                        
 
    	  //적대 태그 확인
        if (!TargetASC->HasMatchingGameplayTag(RSTags::Team_Enemy))                                              
        {                                                                                                      
            continue;                                                                                            
        }                                                                                                     
        // DamageGE 적용 
        FGameplayEffectContextHandle Context = InitData.InstigatorASC->MakeEffectContext();
        FGameplayEffectSpecHandle SpecHandle = InitData.InstigatorASC->MakeOutgoingSpec(InitData.DamageGEClass,1.f, Context);                                                                                                   
                                                                                                                 
        if (!SpecHandle.IsValid())                                                                               
        {                                                                                                      
            KHS_WARN(TEXT("DamageGE 스펙 생성 실패. Target: %s"), *TargetActor->GetName());
            continue;                                                                                            
        }
                                                                                                                 
        SpecHandle.Data->SetSetByCallerMagnitude(RSTags::Data_Damage, -InitData.Amount);                          
        InitData.InstigatorASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
                                                                                                                 
        // StatusGE 적용                                                                             
        if (InitData.StatusGEClass)                                                                              
        {                                                                                                        
            FGameplayEffectSpecHandle StatusSpec = InitData.InstigatorASC->MakeOutgoingSpec(InitData.StatusGEClass, 1.f, InitData.InstigatorASC->MakeEffectContext());                       
                                                                                                                 
            if (StatusSpec.IsValid())                                                                            
            {                                                                                                    
                InitData.InstigatorASC->ApplyGameplayEffectSpecToTarget(*StatusSpec.Data.Get(), TargetASC);      
            }                                                                                                    
        }
    }          
}

