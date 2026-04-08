// Fill out your copyright notice in the Description page of Project Settings.


#include "Objects/Summon/BaseSummonObject.h"
#include "RoastStaffGAS.h"
#include "Engine/OverlapResult.h"
#include "GAS/Tags/RSGameplayTags.h"
#include "Subsystems/PoolingSubsystem.h"
#include "NiagaraComponent.h"

// Sets default values
ABaseSummonObject::ABaseSummonObject()
{
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	VFXComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("VFXComp"));
	VFXComp->SetupAttachment(Root);
	VFXComp->bAutoActivate = false;
}

void ABaseSummonObject::InitSummon(const FSummonObjectInitData& InInitData)
{
	// 풀링 추가로 변경
	// SpawnActorDeferred의 BeginPlay 역할을 InitSummon이 대신 수행                                 
	InitData = InInitData;
	bInitialized = true;
	
	ApplyGameplayEffectToArea();                                                                                 
                                                                                                                   
	const float EffectLifeTime = InitData.Lifetime > 0.f ? InitData.Lifetime : 1.f;                              
	if (InitData.Lifetime <= 0.f)                                                                                
	{                                                                                                            
		KHS_WARN(TEXT("Lifetime 0 이하 — 기본값 1.f 적용. SkillID: %s"),*InitData.SkillID.ToString());                                                                       
	}                                 
	
	GetWorldTimerManager().SetTimer(LifetimeHandle, this, &ABaseSummonObject::OnLifetimeExpired, EffectLifeTime, false);    
}

void ABaseSummonObject::OnPoolActivate()
{
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true); 
	if (VFXComp)
	{                                                                                                            
		VFXComp->Activate(true); // true = 리셋 후 재생                                                          
	}   
}

void ABaseSummonObject::OnPoolDeactivate()
{
	SetActorHiddenInGame(true);                                                                                  
	SetActorEnableCollision(false);
	GetWorldTimerManager().ClearTimer(LifetimeHandle);
	bInitialized = false;  
	if (VFXComp)                                                                                                 
	{                                                                                                          
		VFXComp->Deactivate();
	}     
}

// Called when the game starts or when spawned
void ABaseSummonObject::BeginPlay()
{
	Super::BeginPlay();
	
	//풀링 (최초 스폰시 비활성 상태)
	OnPoolDeactivate();
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

void ABaseSummonObject::OnLifetimeExpired()
{
	GET_WORLD_SUBSYSTEM(UPoolingSubsystem, PoolSys);                                                           
	PoolSys->ReturnToPool(this);  
}

