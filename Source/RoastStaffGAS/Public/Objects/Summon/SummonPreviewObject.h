// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SummonPreviewObject.generated.h"

/**                                                                                                              
  * ASummonPreviewObject                                                                                        
  * 시각 전용 소환 프리뷰 오브젝트. 
  * 매 프레임 PlayerController::CachedAimLocation을 따라 위치 갱신.                                               
  */   

UCLASS()
class ROASTSTAFFGAS_API ASummonPreviewObject : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASummonPreviewObject();

protected:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
