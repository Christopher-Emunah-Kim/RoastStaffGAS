// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/Summon/GA_SummonAimedField.h"
#include "RoastStaffGAS.h"
#include "Character/Player/RSPlayerController.h"
#include "Objects/Data/RSSkillData.h"
#include "Objects/Summon/SummonPreviewObject.h"
#include "Subsystems/EquipmentSubsystem.h"


void UGA_SummonAimedField::OnAbilityActivated(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
    if (!LoadSkillInitData())
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    KHS_INFO(TEXT("OnAbilityActivated. bIsActive: %s"), CheckIsActiveSlot() ? TEXT("true") : TEXT("false")); 
    
    // 수동(Active) 모드: 프리뷰 스폰 + WaitConfirmCancel
    if (CheckIsActiveSlot())
    {
        HandleActiveMode();
        return;
    }
    
    // 자동 모드: CachedAimLocation에 즉시 소환
    const FVector SummonLocation = DetermineSummonLocation();
    if (SummonLocation.IsZero())
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }
    SpawnSummonObject(SummonLocation);
    
    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

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


bool UGA_SummonAimedField::LoadSkillInitData()
{
    const URSSkillData* SkillData = Cast<URSSkillData>(GetCurrentSourceObject());
    if (!ensureMsgf(SkillData, TEXT("SourceObject가 URSSkillData가 아님")))
    {
        return false;
    }

    CachedSkillID = SkillData->SkillID;

    if (!LoadSummonData(CachedExecData, CachedSummonParam))
    {
        return false;
    }
    return true;
}


void UGA_SummonAimedField::HandleActiveMode()
{
    KHS_INFO(TEXT("HandleActiveMode 진입. WaitConfirmCancel 등록 시작."));  
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
    
    UAbilityTask_WaitConfirmCancel* WaitTask =  UAbilityTask_WaitConfirmCancel::WaitConfirmCancel(this);
    WaitTask->OnConfirm.AddDynamic(this, &UGA_SummonAimedField::OnConfirm);
    WaitTask->OnCancel.AddDynamic(this, &UGA_SummonAimedField::OnCancel);
    WaitTask->ReadyForActivation();
    KHS_INFO(TEXT("WaitConfirmCancel 등록 완료.")); 
    // Note: EndAbility는 OnConfirm / OnCancel에서 호출
}

void UGA_SummonAimedField::OnConfirm()
{
    const FVector Location = DetermineSummonLocation();
    KHS_INFO(TEXT("OnConfirm 수신. 소환 위치: %s"), *Location.ToString()); 
    if (!Location.IsZero())
    {
        SpawnSummonObject(Location);
    }
    else
    {
        KHS_WARN(TEXT("AimLocation 무효. 소환 취소. SkillID: %s"), *CachedSkillID.ToString());
    }

    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_SummonAimedField::OnCancel()
{
    KHS_INFO(TEXT("소환 취소됨. SkillID: %s"),  *CachedSkillID.ToString());
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

bool UGA_SummonAimedField::CheckIsActiveSlot() const
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
