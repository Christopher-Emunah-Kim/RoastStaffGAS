// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Player/LobbyCharacterActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "System/LoggingSystem.h"

ALobbyCharacterActor::ALobbyCharacterActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// 마우스 호버/클릭 트레이스(Visibility 채널) — 캡슐 + 메시 모두 히트되도록 보장
	// 캡슐만으로는 팔/다리 등 넓은 영역 클릭이 누락됨
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
}

void ALobbyCharacterActor::NotifyActorOnClicked(FKey ButtonPressed)
{
	Super::NotifyActorOnClicked(ButtonPressed);

	if (StandUpMontage)
	{
		PlayAnimMontage(StandUpMontage);
	}

	OnCharacterClickedDel.Broadcast(CharacterID);
}

void ALobbyCharacterActor::SetOutlineActive(bool bActive)
{
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (!MeshComp)
	{
		KHS_WARN("SkeletalMesh 없음 — 아웃라인 설정 불가");
		return;
	}

	MeshComp->SetCustomDepthStencilValue(1);
	MeshComp->SetRenderCustomDepth(bActive);
}
