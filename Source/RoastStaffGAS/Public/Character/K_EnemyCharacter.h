// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/K_BaseCharacter.h"
#include "UI/EnemyStateWidget.h"
#include "K_EnemyCharacter.generated.h"


struct FGameplayEventData;
class UWidgetComponent;
class UEnemyStateWidget;
/**
 * 
 */
UCLASS()
class ROASTSTAFFGAS_API AK_EnemyCharacter : public AK_BaseCharacter
{
	GENERATED_BODY()
	
public:
	AK_EnemyCharacter();
	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void PostInitializeComponents() override;
	virtual void Tick(float DeltaSeconds) override;
	
	//ASC 초기화 및 설정
	virtual void InitializeAbilitySystem() override;
	
	//간단한 AI로직
	void TickSimpleAI(float DeltaTime);
	
	//==============================================
	//이벤트 핸들러
	//==============================================
	//사망 이벤트 핸들러(Event.Combat.Death)
	void OnDeathEvent(const FGameplayEventData* Payload);
	
	//데미지 이벤트 핸들러
	void OnTakeDamageEvent(const FGameplayEventData* Payload);
	
	//==============================================
	//이벤트 처리
	//==============================================
	//사망 처리
	UFUNCTION(BlueprintNativeEvent, Category = "AM|Combat")
	void HandleDeath();
	virtual void HandleDeath_Implementation();
	
	//액터 파괴
	UFUNCTION()
	void DestroyAfterDeath();
	
	//데미지 처리
	UFUNCTION(BlueprintNativeEvent, Category = "AM|Combat")
	void HandleDamageFeedback();
	virtual void HandleDamageFeedback_Implementation();
	
	//==============================================
	//UI 처리
	//==============================================
	void InitializeStateWidget();
	
private:
	FTimerHandle DamageFeedbackTimer;
	FTimerHandle DeathDestroyTimer;
	
protected:
	//Comps
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AM|Comp")
	TObjectPtr<UWidgetComponent> StateWidgetComp;
	
	//Widget Class
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AM|UI")
	TSubclassOf<UEnemyStateWidget> StateWidgetClass;
	
	//Widget Instance
	UPROPERTY()
	TObjectPtr<UEnemyStateWidget> StateWidget;
	
	//Setting
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AM|Combat")
	float DeathDestroyDelay = 3.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AM|Combat")
	float DamageFeedbackDuration = 0.1f;
	
	//state
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AM|Combat")
	bool bIsDead = false;
	
	//Simple Trace
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AM|AI")
	float DetectionRange = 1000.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AM|AI")
	float MoveSpeed = 50.f;
	
};
