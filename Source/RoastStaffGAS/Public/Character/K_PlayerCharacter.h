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
/**
 * 
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
	
public:
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	
protected:
	void OnMove(const FInputActionValue& Value);
	void OnMouseAim(const FInputActionValue& Value);
	void OnDash(const FInputActionValue& Value);
	void OnShootStart(const FInputActionValue& Value);
	void OnShootStop(const FInputActionValue& Value);
	void OnFireballAttack(const FInputActionValue& Value);
	
	void DoShoot();
	
	
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AM|Comp", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AM|Comp", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* Camera;
	
protected:
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
	
	//플레이어 컨트롤러
	UPROPERTY()
	TObjectPtr<AK_PlayerController> KPlayerController;
	
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
	
	//발사체 클래스
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AM|Shooting")
	TSubclassOf<ATwinStickProjectile> ProjectileClass;
	
	//발사체 발사 오프셋
	UPROPERTY(EditAnywhere, Category="AM|Shooting", meta = (ClampMin = 0, ClampMax = 1000, Units = "cm"))
	float ProjectileOffset = 100.0f;
	//AutoFire 타이머핸들
	FTimerHandle AutoFireTimer;
	
	//AutoFire 딜레이 시간
	UPROPERTY(EditAnywhere, Category="AM|Shooting", meta = (ClampMin = 0, ClampMax = 5, Units = "s"))
	float AutoFireDelay = 0.2f;
	
	//Fireball 어빌리티 클래스
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AM|GAS|Ability")
	TSubclassOf<UGameplayAbility> FireballAbilityClass;
	
	//Fireball 쿨타임
	UPROPERTY(EditAnywhere, Category="AM|Fireball", meta = (ClampMin = 0, ClampMax = 10, Units = "s"))
	float FireballCooldownTime = 1.0f;
	
	//Fireball 경과시간
	float LastFireballAttackTime = 0.f;
	
};
