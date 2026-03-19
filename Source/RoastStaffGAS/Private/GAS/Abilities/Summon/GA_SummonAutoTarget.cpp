// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/Summon/GA_SummonAutoTarget.h"
#include "RoastStaffGAS.h"
#include "Character/BaseCharacter.h"
#include "GAS/Tags/RSGameplayTags.h"
#include "Objects/Data/RSSkillData.h"
#include "Subsystems/EquipmentSubsystem.h"


FVector UGA_SummonAutoTarget::DetermineSummonLocation()
{
	AActor* NearestEnemy = nullptr;
	float   NearestDistSq = FLT_MAX;
	
	const float SearchRange = FindNearestEnemy(NearestEnemy, NearestDistSq);

	if (!NearestEnemy)
	{
		KHS_INFO(TEXT("SearchRange(%.0f) 내 적 없음. SkillID: %s"),	SearchRange, *CachedSkillID.ToString());
		return CachedInstigator->GetActorLocation() + CachedInstigator->GetActorForwardVector() * 1000.f;  
	}

	KHS_INFO(TEXT("FIND ENEMY. SKillID : %s, 적과의 거리 : %.0f"), *CachedSkillID.ToString(), NearestDistSq);
	return NearestEnemy->GetActorLocation();

}

float UGA_SummonAutoTarget::FindNearestEnemy(AActor*& NearestEnemy, float& NearestDistSq)
{
	const float SearchRange = CachedSummonParam.SearchRange;
	
	const FVector Origin = GetOwningActorFromActorInfo()->GetActorLocation();
	TArray<FOverlapResult> Overlaps;
	const FCollisionShape Sphere = FCollisionShape::MakeSphere(SearchRange);
	GetWorld()->OverlapMultiByChannel(Overlaps, Origin, FQuat::Identity, ECC_Pawn, Sphere);
	
	//SearchRange내에 적대태그 가진 유효 적이 있을때 해당 위치 반환. 없으면 기본값 반환.
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Candidate = Overlap.GetActor();
		if (!Candidate)
		{
			continue;
		}

		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Candidate);
		if (!TargetASC || !TargetASC->HasMatchingGameplayTag(RSTags::Team_Enemy))
		{
			continue;
		}

		const float DistSq = FVector::DistSquared(Origin, Candidate->GetActorLocation());
		if (DistSq < NearestDistSq)
		{
			NearestDistSq = DistSq;
			NearestEnemy  = Candidate;
		}
	}
	return SearchRange;
}
