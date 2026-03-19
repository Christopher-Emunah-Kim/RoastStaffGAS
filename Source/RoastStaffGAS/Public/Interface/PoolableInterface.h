// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PoolableInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UPoolableInterface : public UInterface
{
	GENERATED_BODY()
};

/**                                                                                                            
   * IRSPoolableInterface
   * 오브젝트 풀링 가능한 액터가 구현해야 하는 인터페이스.
   * AActor 파생 클래스 (SummonObject, Projectile, Enemy 등) 모두 적용 가능.                                       
   * UUserWidget 파생용 풀링은 별도 채널로 확장.                                                 
   */  

class ROASTSTAFFGAS_API IPoolableInterface
{
	GENERATED_BODY()
public:                                                                                                        
	/** 풀에서 꺼낼 때 호출 — 가시성·충돌 활성화 */
	virtual void OnPoolActivate() = 0;                                                                           
	/** 풀에 반납할 때 호출 — 가시성·충돌 비활성화, 상태 초기화 */
	virtual void OnPoolDeactivate() = 0;   
	
};
