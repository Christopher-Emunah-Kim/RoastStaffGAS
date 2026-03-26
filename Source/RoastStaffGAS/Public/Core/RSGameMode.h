// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "RSGameMode.generated.h"

/**
 * ARSGameMode
 *
 * - 스테이지 시작 진입점. 초기화 순서를 보장하는 조율자 역할만 담당한다.
 * - Pool 초기화: UStageManagerSubsystem::StartStage → AEnemySpawner::InitPools()로 이관.
 * - 웨이브/스폰 로직: UStageManagerSubsystem에 위임.
 */
UCLASS()
class ROASTSTAFFGAS_API ARSGameMode : public AGameModeBase
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

private:
	/** DT_Stage 행 이름과 일치해야 함 */
	UPROPERTY(EditDefaultsOnly, Category = "MY|Stage")
	FName DefaultStageID = TEXT("STG_001");
};
