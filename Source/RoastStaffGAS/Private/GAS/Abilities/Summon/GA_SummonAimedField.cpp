// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/Summon/GA_SummonAimedField.h"
#include "RoastStaffGAS.h"
#include "Character/Player/RSPlayerController.h"
#include "Objects/Data/RSSkillData.h"
#include "Objects/Summon/SummonPreviewObject.h"
#include "Subsystems/EquipmentSubsystem.h"


void UGA_SummonAimedField::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,	bool bReplicateEndAbility, bool bWasCancelled)
{
    KHS_INFO(TEXT("EndAbility 진입. bWasCancelled: %s"), bWasCancelled ? TEXT("true") : TEXT("false")); 
    // 프리뷰 항상 제거 (확인/취소/강제 종료 모두 대응)
    if (CachedPreviewObject.Get())
    {
        CachedPreviewObject->Destroy();
        CachedPreviewObject = nullptr;
    }
    
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

FVector UGA_SummonAimedField::DetermineSummonLocation()
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


void UGA_SummonAimedField::HandleActiveMode()
{
    //프리뷰 소환 후 공통 로직
    SpawnPreviewObject();
    
    Super::HandleActiveMode();
}

void UGA_SummonAimedField::SpawnPreviewObject()
{
    TSubclassOf<AActor> PreviewClass;
    if (LoadRequiredClass(CachedExecData.SummonPreviewClass, PreviewClass, CachedSkillID))
    {
        ARSPlayerController* PC =  Cast<ARSPlayerController>(GetWorld()->GetFirstPlayerController());
        const FVector InitLocation = PC ? PC->GetCachedAimLocation() :  FVector::ZeroVector;

        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = GetOwningActorFromActorInfo();
        CachedPreviewObject = GetWorld()->SpawnActor<ASummonPreviewObject>(PreviewClass, InitLocation, FRotator::ZeroRotator, SpawnParams);
    }
    else
    {
        KHS_WARN(TEXT("SummonPreviewClass 로드 실패. 프리뷰 없이 진행 SkillID: %s"),  *CachedSkillID.ToString());
    }
}