// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/EquipmentComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Subsystems/GameDataSubsystem.h"
#include "Data/RuntimeDataStructs.h"
#include "GAS/Tags/RSGameplayTags.h"
#include "System/LoggingSystem.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

UEquipmentComponent::UEquipmentComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	for (int32 i = 0; i < SLOT_COUNT; ++i)
	{
		Slots[i].SlotIndex = i;
	}
}

void UEquipmentComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UEquipmentComponent::FireSlot(int32 SlotIndex)
{
    FWeaponSlotInstanceData& Slot = Slots[SlotIndex];
    
    // GA 트리거 이벤트 전송 — 에임 좌표를 페이로드에 포함
    FGameplayEventData Payload;
    Payload.Instigator = GetOwner();
    Payload.Target     = GetOwner();
    Payload.TargetData = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(GetOwner());

    // 에임 월드 좌표를 EventMagnitude 대신 TargetData로 전달
    // TODO: FGameplayAbilityTargetData_SingleTargetHit으로 에임 좌표 포함 — GA 구현 시 확정
    UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
        GetOwner(), RSTags::Event_Weapon_Fire,Payload
    );

    // 쿨타임 시작
    Slot.CooldownRemaining = Slot.EquipData.Cooldown;
    GetWorld()->GetTimerManager().SetTimer(
        Slot.AutoFireTimerHandle,
        [this, SlotIndex]()
        {
            Slots[SlotIndex].CooldownRemaining = 0.f;
            
            if (!Slots[SlotIndex].bIsActive)
            {
                FireSlot(SlotIndex); // 자동공격: 쿨타임 만료 시 즉시 재발사
            }
        },
        Slot.EquipData.Cooldown, false
    );

    KHS_INFO(TEXT("Slot %d: %s 발사! CD: %.2fs"),
        SlotIndex, *Slot.EquipData.SkillID.ToString(), Slot.EquipData.Cooldown);
}

void UEquipmentComponent::StartAutoFire(int32 SlotIndex)
{
    FWeaponSlotInstanceData& Slot = Slots[SlotIndex];

    GetWorld()->GetTimerManager().SetTimer(
        Slot.AutoFireTimerHandle,
        [this, SlotIndex]()
        {
            if (!Slots[SlotIndex].bIsActive)
            {
                FireSlot(SlotIndex);
            }
        },
        Slot.EquipData.Cooldown, true, 0.f // 즉시 1회 후 반복
    );

    KHS_INFO(TEXT("Slot %d: 자동공격 타이머 시작. CD: %.2fs"),
        SlotIndex, Slot.EquipData.Cooldown);
}

void UEquipmentComponent::StopAutoFire(int32 SlotIndex)
{
    GetWorld()->GetTimerManager().ClearTimer(Slots[SlotIndex].AutoFireTimerHandle);

}

void UEquipmentComponent::SetSlotActive(int32 SlotIndex)
{
    // 기존 액티브 슬롯 자동공격 복귀
    if (ActiveSlotIndex != -1)
    {
        Slots[ActiveSlotIndex].bIsActive = false;
        StartAutoFire(ActiveSlotIndex);
    }

    // 새 슬롯 액티브 전환
    Slots[SlotIndex].bIsActive = true;
    ActiveSlotIndex = SlotIndex;
    StopAutoFire(SlotIndex);

    KHS_INFO(TEXT("Slot %d → 액티브 모드 전환."), SlotIndex);
}

void UEquipmentComponent::ClearActiveSlot()
{
    if (ActiveSlotIndex == -1)
    {
        return;
    }

    Slots[ActiveSlotIndex].bIsActive = false;
    StartAutoFire(ActiveSlotIndex);

    KHS_INFO(TEXT("Slot %d → 자동공격 복귀."), ActiveSlotIndex);

    ActiveSlotIndex = -1;
}

FVector UEquipmentComponent::GetAimWorldLocation() const
{
    APlayerController* PC = Cast<APlayerController>(Cast<APawn>(GetOwner())->GetController());

    if (!PC)
    {
        return GetOwner()->GetActorForwardVector() * 1000.f + GetOwner()->GetActorLocation();
    }

    FHitResult Hit;
    PC->GetHitResultUnderCursor(ECC_Visibility, false, Hit);
    
    return Hit.bBlockingHit ? Hit.Location : GetOwner()->GetActorLocation();
}

bool UEquipmentComponent::IsValidSlotIndex(int32 SlotIndex) const
{
    return SlotIndex >= 0 && SlotIndex < SLOT_COUNT;
}

void UEquipmentComponent::InitializeWithASC(UAbilitySystemComponent* InASC)
{
	check(InASC);
	ASC = InASC;
}

void UEquipmentComponent::EquipWeapon(const FName& WeaponID)
{
	check(ASC);

    // GDS에서 장착에 필요한 모든 데이터를 한 번에 조회
    UGameInstance* GI = GetWorld()->GetGameInstance();
    check(GI);
    UGameDataSubsystem* GDS = GI->GetSubsystem<UGameDataSubsystem>();
    check(GDS);

    FWeaponEquipData EquipData;
    if (!GDS->GetWeaponEquipData(WeaponID, EquipData))
    {
        KHS_WARN(TEXT("WeaponID 조회 실패: %s"), *WeaponID.ToString());
        return;
    }

    int32 TargetSlot = -1;
    for (int32 i = 0; i < SLOT_COUNT; ++i)
    {
        if (Slots[i].IsEmpty())
        {
            TargetSlot = i;
            break;
        }
    }

    if (TargetSlot == -1)
    {
        KHS_INFO(TEXT("빈 슬롯 없음. 무기 획득 불가: %s"), *WeaponID.ToString());
        return;
    }

    // 클래스 동기 로드
    TSubclassOf<UGameplayAbility> GAClass = EquipData.GAClass.LoadSynchronous();
    if (!ensureMsgf(GAClass, TEXT("GA 클래스 로드 실패: %s"), *WeaponID.ToString()))
    {
        return;
    }

    TSubclassOf<UGameplayEffect> DamageGEClass = EquipData.DamageGEClass.LoadSynchronous();
    if (!ensureMsgf(DamageGEClass, TEXT("DamageGE 클래스 로드 실패: %s"), *WeaponID.ToString()))
    {
        return;
    }

    // StatusGE — 없을 수 있음 (빈 값 허용)
    TSubclassOf<UGameplayEffect> StatusGEClass = nullptr;
    if (!EquipData.StatusGEClass.IsNull())
    {
        StatusGEClass = EquipData.StatusGEClass.LoadSynchronous();
        
        if (!StatusGEClass)
        {
            KHS_WARN(TEXT("StatusGE 로드 실패. 스킵: %s"), *WeaponID.ToString());
        }
    }

    // 투사체 클래스 — 소환형은 nullptr 허용
    TSubclassOf<AActor> ProjectileClass = nullptr;
    if (!EquipData.ProjectileClass.IsNull())
    {
        ProjectileClass = EquipData.ProjectileClass.LoadSynchronous();
        
        if (!ProjectileClass)
        {
            KHS_WARN(TEXT("투사체 클래스 로드 실패: %s"), *WeaponID.ToString());
            check(false);
            return;
        }
    }

    // ASC에 GA 부여
    FGameplayAbilitySpec Spec(GAClass, 1, INDEX_NONE, GetOwner());
    FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(Spec);
    if (!Handle.IsValid())
    {
        KHS_WARN(TEXT("GiveAbility 실패: %s"), *WeaponID.ToString());
        check(false);
        return;
    }

    // 슬롯 런타임 데이터 기록
    FWeaponSlotInstanceData& Slot = Slots[TargetSlot];
    Slot.EquipData.WeaponID             = WeaponID;
    Slot.EquipData.SkillID              = EquipData.SkillID;
    Slot.AbilitySpecHandle              = Handle;
    Slot.EquipData.Cooldown             = EquipData.Cooldown;
    Slot.CooldownRemaining              = 0.f;
    Slot.bIsActive                      = false;
    Slot.EquipData.GAClass              = GAClass;
    Slot.EquipData.ProjectileClass      = ProjectileClass;
    Slot.EquipData.DamageGEClass        = DamageGEClass;
    Slot.EquipData.StatusGEClass        = StatusGEClass;

    KHS_INFO(TEXT("무기 장착 완료: %s → Slot %d"), *WeaponID.ToString(), TargetSlot);

    StartAutoFire(TargetSlot);
}

void UEquipmentComponent::RequestSlotActivate(int32 SlotIndex)
{
    if (!IsValidSlotIndex(SlotIndex))
    {
        return;
    }

    if (Slots[SlotIndex].IsEmpty())
    {
        KHS_INFO(TEXT("Slot %d 비어있음. 입력 무시."), SlotIndex);
        return;
    }

    // 이미 액티브 슬롯이면 해제
    if (ActiveSlotIndex == SlotIndex)
    {
        ClearActiveSlot();
        return;
    }

    SetSlotActive(SlotIndex);
}

void UEquipmentComponent::OnAttackInput()
{
    if (ActiveSlotIndex == -1)
    {
        return; // 액티브 슬롯 없음 — 무시
    }

    FWeaponSlotInstanceData& Slot = Slots[ActiveSlotIndex];

    FireSlot(ActiveSlotIndex); //액티브는 쿨타임없이 발사가능.
}

