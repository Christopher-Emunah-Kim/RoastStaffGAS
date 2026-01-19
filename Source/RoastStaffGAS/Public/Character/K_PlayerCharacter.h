// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/K_BaseCharacter.h"
#include "K_PlayerCharacter.generated.h"

class AK_PlayerController;
class ATwinStickProjectile;
class UInputAction;
class UGameplayAbility;
class USpringArmComponent;
class UCameraComponent;
struct FInputActionValue;
class AK_PlayerState;

/**
 * K_PlayerCharacter
 * 
 * 플레이어가 조종하는 캐릭터 클래스.
 * 
 * ASC 소유권: PlayerState
 * - 이 클래스는 Avatar 역할만 수행
 * - ASC는 PlayerState에서 소유하고 관리
 * - GetAbilitySystemComponent()는 PlayerState의 ASC를 반환
 * 
 * 초기화 타이밍:
 * - Server: PossessedBy()에서 PlayerState 확보 후 InitAbilityActorInfo 호출
 * - Client: OnRep_PlayerState()에서 PlayerState 리플리케이션 후 InitAbilityActorInfo 호출
 */

UCLASS()
class ROASTSTAFFGAS_API AK_PlayerCharacter : public AK_BaseCharacter
{
	GENERATED_BODY()
	
	AK_PlayerCharacter();
	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void PostInitializeComponents() override;
	//Server - Controller가 Pawn을 possess할때 
	virtual void PossessedBy(AController* NewController) override;
	//Client - PlayerState가 리플리케이트 될때.
	virtual void OnRep_PlayerState() override;
	
public:
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual UK_BaseAttributeSet* GetAttributeSet() const override;
	virtual void InitializeAbilitySystem() override;
	
protected:
	//입력 핸들러
	void OnMove(const FInputActionValue& Value);
	void OnMouseAim(const FInputActionValue& Value);
	void OnDash(const FInputActionValue& Value);
	void OnShootStart(const FInputActionValue& Value);
	void OnShootStop(const FInputActionValue& Value);
	void OnFireballAttack(const FInputActionValue& Value);
	
	//GAS 기반 Activate 함수
	UFUNCTION(BlueprintCallable, Category="AM|Movement")
	void TryActivateDash();
	
	UFUNCTION(BlueprintCallable, Category="AM|Combat")
	void TryActivateBasicShoot();
	
	UFUNCTION(BlueprintCallable, Category="AM|Combat")
	void TryActivateFireball();
	
	
	
	AK_PlayerState* GetKPlayerState() const;
	
public:
	FVector GetDashDirection() const;
	
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AM|Comp", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AM|Comp", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* Camera;
	
protected:
	//============================================================================
	//입력처리
	//============================================================================
	//Input Actions
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AM|Input")
	UInputAction* IA_Move;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AM|Input")
	UInputAction* IA_MouseAim;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AM|Input")
	UInputAction* IA_Dash;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AM|Input")
	UInputAction* IA_Shoot;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AM|Input")
	UInputAction* IA_FireBall;
	
	//============================================================================
	//레퍼런스
	//============================================================================
	//플레이어 컨트롤러 레퍼런스
	UPROPERTY()
	TObjectPtr<AK_PlayerController> KPlayerController;
	
	//============================================================================
	//이동/에임
	//============================================================================
	//마지막 이동입력 캐싱
	FVector2D LastMoveInput;
	
	//대시 파워
	UPROPERTY(EditAnywhere, Category="AM|Dash", meta = (ClampMin = 0, ClampMax = 10000, Units = "cm/s"))
	float DashImpulse = 2500.0f;
	
	//마우스 에임 각도
	float AimAngle = 0.f;
	
	//마우스 에임 채널
	UPROPERTY(EditAnywhere, Category="AM|Input")
	TEnumAsByte<ETraceTypeQuery> MouseAimTraceChannel;
	
	
	//============================================================================
	// 공격
	//============================================================================
	//AutoFire 타이머핸들
	FTimerHandle AutoFireTimer;
	
	//AutoFire 딜레이 시간
	UPROPERTY(EditAnywhere, Category="AM|Shooting", meta = (ClampMin = 0, ClampMax = 5, Units = "s"))
	float AutoFireDelay = 0.2f;
	
	//Fireball 쿨타임
	UPROPERTY(EditAnywhere, Category="AM|Fireball", meta = (ClampMin = 0, ClampMax = 10, Units = "s"))
	float FireballCooldownTime = 1.0f;
	
	//Fireball 경과시간
	float LastFireballAttackTime = 0.f;
	
};
