// Fill out your copyright notice in the Description page of Project Settings.


#include "Objects/Projectile/BaseProjectile.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameplayEffect.h"
#include "RoastStaffGAS.h"
#include "GAS/Tags/RSGameplayTags.h"
#include "Subsystems/PoolingSubsystem.h"
#include "System/LoggingSystem.h"
#include "Data/EnumTypes.h"

ABaseProjectile::ABaseProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetSphereRadius(200.f);
	SphereComp->SetCollisionProfileName(TEXT("Projectile"));
	RootComponent = SphereComp;

	ProjectileComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileComp->InitialSpeed  = 1000.f;
	ProjectileComp->MaxSpeed      = 3000.f;
	ProjectileComp->bRotationFollowsVelocity = true;
	ProjectileComp->ProjectileGravityScale   = 0.f;
}

void ABaseProjectile::OnPoolActivate()
{
	ProjectileComp->SetUpdatedComponent(GetRootComponent());
	
	SetActorHiddenInGame(false);                                                                                 
	SetActorEnableCollision(true); 
}

void ABaseProjectile::OnPoolDeactivate()
{
	ProjectileComp->StopMovementImmediately();
	ProjectileComp->Velocity = FVector::ZeroVector;
	ProjectileComp->bIsHomingProjectile = false;
	ProjectileComp->HomingTargetComponent = nullptr;
	ProjectileComp->ProjectileGravityScale = 0.f;

	SphereComp->ClearMoveIgnoreActors();
	bHasPierceFinished = false;
	PierceHitCount = 0;
	BounceHitCount = 0;

	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	GetWorldTimerManager().ClearTimer(LifetimeTimerHandle);
}

void ABaseProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	SphereComp->OnComponentHit.AddDynamic(this, &ABaseProjectile::OnHit);
	SphereComp->OnComponentBeginOverlap.AddDynamic(this, &ABaseProjectile::OnBeginOverlap);
	OnPoolDeactivate();
}


void ABaseProjectile::InitProjectile(const FProjectileInitData& InInitData)
{
	// 풀 재사용 대비 상태 리셋
	bHasExploded = false;
	bHasPierceFinished = false;
	PierceHitCount = 0;
	SphereComp->ClearMoveIgnoreActors();

	// 투사체 기본 초기화
	InitData = InInitData;

	// 발사자(자신)와의 충돌을 물리적으로 무시하도록 예외 처리 추가
	if (InitData.InstigatorASC && InitData.InstigatorASC->GetAvatarActor())
	{
		AActor* AvatarActor = InitData.InstigatorASC->GetAvatarActor();
		SphereComp->IgnoreActorWhenMoving(AvatarActor, true); // 발사자를 뚫고 나가도록 설정
        
		SetOwner(AvatarActor); 
		SetInstigator(Cast<APawn>(AvatarActor));
	}
	
	// 속도 세팅
	ProjectileComp->InitialSpeed = InitData.Speed;
	ProjectileComp->MaxSpeed     = InitData.Speed;
	ProjectileComp->Velocity     = GetActorForwardVector() * InitData.Speed;
	ProjectileComp->UpdateComponentVelocity();
	ProjectileComp->Activate();

	// 수명 타이머
	const float SafeLifetime = InitData.Lifetime > 0.f ? InitData.Lifetime : 5.f;
	if (InitData.Lifetime <= 0.f)
	{
		KHS_WARN(TEXT("Lifetime이 0 이하 — 기본값 5.f 적용"));
	}

	GetWorldTimerManager().SetTimer(LifetimeTimerHandle,	this,
		&ABaseProjectile::OnLifetimeExpired,SafeLifetime,false);

	// 자식 타입별 추가 초기화
	OnProjectileInitialized();
}


void ABaseProjectile::OnProjectileInitialized()
{
	//HitType별 추가 처리
	switch(InitData.HitType)
	{
	case EHitType::PIERCE:
		{
			// PIERCE: Pawn 채널을 Overlap으로 변경하여 적을 통과하며 OnBeginOverlap으로 감지
			SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		}
		break;
		
	default:
		{
			//추가 처리이므로 별도 디폴트 처리 없음.
		}
	}

	//MoveType별 추가 처리
	switch (InitData.MoveType)
	{
	case EMoveType::HOMING:
		{
			if (InitData.HomingTarget.IsValid())
			{
				ProjectileComp->bIsHomingProjectile = true;
				ProjectileComp->HomingTargetComponent = InitData.HomingTarget.Get();
				ProjectileComp->HomingAccelerationMagnitude = InitData.TurnSpeed;
			}
			else
			{
				KHS_WARN(TEXT("HOMING 타겟 없음 — 직선 비행 폴백. SkillID: %s"), *InitData.SkillID.ToString());
			}
		}
		break;
	case EMoveType::HOMING_BOUNCE:
		{
			// Pawn 채널 Overlap으로 변경 — OnHit 대신 HandleBounceHit으로 처리
			SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

			// 첫 타겟: InitData.HomingTarget 우선, 없으면 전방 가장 가까운 적 탐색
			if (InitData.HomingTarget.IsValid())
			{
				ProjectileComp->bIsHomingProjectile = true;
				ProjectileComp->HomingTargetComponent = InitData.HomingTarget.Get();
				ProjectileComp->HomingAccelerationMagnitude = InitData.TurnSpeed;
			}
			else
			{
				KHS_WARN(TEXT("HOMING_BOUNCE 초기 타겟 없음 — 직선 비행. SkillID: %s"), *InitData.SkillID.ToString());
			}
		}
		break;
	case EMoveType::ARC:
		{
			// RotateAngleAxis로 짐벌락 없이 LaunchAngle만큼 상방 회전
			const FVector RightVec = FVector::CrossProduct(GetActorForwardVector(), FVector::UpVector).GetSafeNormal();
			const FVector LaunchVel = GetActorForwardVector().RotateAngleAxis(InitData.LaunchAngle, RightVec) * InitData.Speed;

			if (LaunchVel.IsNearlyZero())
			{
				KHS_WARN(TEXT("ARC LaunchAngle 계산 Zero — 에임 방향 폴백. SkillID: %s"), *InitData.SkillID.ToString());
			}
			else
			{
				ProjectileComp->Velocity = LaunchVel;
			}

			ProjectileComp->ProjectileGravityScale = InitData.GravityScale;
		}
		break;
		
	default:
		{
			//추가 처리이므로 별도 디폴트 없음.
		}
		break;
	}
}

void ABaseProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!OtherActor || OtherActor == GetOwner())
	{
		return;
	}

	switch (InitData.HitType)
	{
	case EHitType::AREA: // 폭발 공격 - 착탄 위치에서 범위 폭발 (벽/지형 포함 충돌대상 무관. AREA 타입)
		{
			HandleAreaHit(Hit.ImpactPoint);
			ReturnToPool();
			return;
		}
	case EHitType::PIERCE: // 관통 공격 — OnBeginOverlap에서 처리. 벽 blocking hit는 무시
		{
			return;
		}
	case EHitType::SINGLE: // 일반 공격 처리
		{
			HandleHitEvent(OtherActor, Hit);
			break;
		}

	default:
		{
			HandleHitEvent(OtherActor, Hit);
			break;
		}
	}
	
}


void ABaseProjectile::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == GetOwner())
	{
		return;
	}

	if (InitData.HitType == EHitType::PIERCE)
	{
		HandlePierceHit(OtherActor);
	}
	else if (InitData.MoveType == EMoveType::HOMING_BOUNCE)
	{
		HandleBounceHit(OtherActor, SweepResult);
	}
}


void ABaseProjectile::ReturnToPool()
{
	GET_WORLD_SUBSYSTEM(UPoolingSubsystem, PoolSys);
	PoolSys->ReturnToPool(this);
}


void ABaseProjectile::ApplyMultipleEffectsToTarget(UAbilitySystemComponent* TargetASC, float DamageMultiplier)
{
	if (InitData.DamageGEClass)
	{
		ApplyEffectToTarget(TargetASC, InitData.DamageGEClass, InitData.Amount * DamageMultiplier);
	}

	if (InitData.StatusGEClass)
	{
		ApplyEffectToTarget(TargetASC, InitData.StatusGEClass, 0.f);
	}
}

void ABaseProjectile::ApplyEffectToTarget(UAbilitySystemComponent* TargetASC,  TSubclassOf<UGameplayEffect> EffectClass,
	float DamageValue)
{
	if (!TargetASC || !EffectClass || !InitData.InstigatorASC)
	{
		KHS_WARN(TEXT("유효하지 않은 파라미터"));
		return;
	}
	
	FGameplayEffectContextHandle Context = InitData.InstigatorASC->MakeEffectContext();
	Context.AddInstigator(GetInstigator(), this);

	FGameplayEffectSpecHandle Spec = InitData.InstigatorASC->MakeOutgoingSpec(EffectClass, 1.f, Context);

	if (!Spec.IsValid())
	{
		KHS_WARN(TEXT("GE Spec 생성 실패"));
		return;
	}

	// ExecCalc(RS_DamageExecCalc)이 읽는 플레이어 기본 데미지 키
	if (DamageValue > 0.f)
	{
		Spec.Data->SetByCallerTagMagnitudes.Add(RSTags::Data_WeaponBaseDamage, DamageValue);
	}

	InitData.InstigatorASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
}


void ABaseProjectile::OnProjectileExpired()
{
	// ARC 미착탄 수명 만료
	if (InitData.HitType == EHitType::AREA)
	{
		HandleAreaHit(GetActorLocation());
	}
	// ReturnToPool은 OnLifetimeExpired()가 담당
}


void ABaseProjectile::HandleHitEvent(AActor* OtherActor, const FHitResult& Hit)
{
	// Enemy ASC 보유 오브젝트에만 반응
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
	if (!TargetASC) //GAS오브젝트가 아님.
	{
		return;
	}
	
	// 자식 충돌 처리 — true 반환 시 베이스가 GE처리하고 ReturnPool
	const bool bHasEnemyTag = TargetASC->HasMatchingGameplayTag(RSTags::Team_Enemy);
	const bool bShouldDestroy = OnProjectileHit(OtherActor, Hit);

	if (bHasEnemyTag && bShouldDestroy)
	{
		ApplyMultipleEffectsToTarget(TargetASC, 1.f);
		ReturnToPool();
	}
}

void ABaseProjectile::HandleAreaHit(const FVector& Center)
{
	if (bHasExploded)
	{
		return;
	}
	bHasExploded = true;
	
	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(GetInstigator());

	GetWorld()->OverlapMultiByChannel(Overlaps, Center,FQuat::Identity,ECC_Pawn,
		FCollisionShape::MakeSphere(InitData.HitRadius), QueryParams
	);

	TSet<AActor*> HitActors;
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* OverlappedActor = Overlap.GetActor();
		if (!OverlappedActor || HitActors.Contains(OverlappedActor))
		{
			continue;
		}

		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OverlappedActor);
		if (!TargetASC || !TargetASC->HasMatchingGameplayTag(RSTags::Team_Enemy))
		{
			continue;
		}

		HitActors.Add(OverlappedActor); //한번 때린 적은 무시

		// 거리 기반 데미지 감쇠율 (Area 판정 규칙)
		const float Dist = FVector::Dist(Center, OverlappedActor->GetActorLocation());
		const float Ratio = (InitData.HitRadius > 0.f) ? (Dist / InitData.HitRadius) : 0.f;

		float DamageMultiplier = 1.f;
		if (Ratio >= 0.7f)
		{
			DamageMultiplier = 0.4f;
		}
		else if (Ratio >= 0.3f)
		{
			DamageMultiplier = 0.7f;
		}

		ApplyMultipleEffectsToTarget(TargetASC, DamageMultiplier);
	}
	
	
}

void ABaseProjectile::HandlePierceHit(AActor* OtherActor)
{
	if (bHasPierceFinished)
	{
		return;
	}

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
	if (!TargetASC)
	{
		return; // 벽/지형 — ignore 추가 없이 통과
	}

	if (!TargetASC->HasMatchingGameplayTag(RSTags::Team_Enemy))
	{
		return;
	}

	SphereComp->IgnoreActorWhenMoving(OtherActor, true); //한번 충돌한 액터는 무시
	++PierceHitCount; // MoveIgnoreActors와 분리 — 발사자 등록 영향 없음

	const float Multiplier = FMath::Max(0.f, 1.f - (PierceHitCount - 1) * InitData.DamageDecay);

	if (Multiplier > 0.f)
	{
		ApplyMultipleEffectsToTarget(TargetASC, Multiplier);
	}

	if (PierceHitCount >= InitData.PierceCount)
	{
		bHasPierceFinished = true;
		ReturnToPool();
	}
}

void ABaseProjectile::HandleBounceHit(AActor* OtherActor, const FHitResult& Hit)
{
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
	if (!TargetASC || !TargetASC->HasMatchingGameplayTag(RSTags::Team_Enemy))
	{
		return;
	}

	// 동일 액터 중복 히트 방지
	SphereComp->IgnoreActorWhenMoving(OtherActor, true);

	// 데미지 적용
	ApplyMultipleEffectsToTarget(TargetASC, 1.f);
	++BounceHitCount;

	// 최대 바운스 횟수 도달 시 소멸
	if (BounceHitCount >= MAX_BOUNCE_COUNT)
	{
		ReturnToPool();
		return;
	}

	// 다음 가장 가까운 적 탐색 (이미 맞은 액터 제외)
	const FVector CurrentLoc = GetActorLocation();
	constexpr float BounceSearchRadius = 1500.f;

	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(GetInstigator());

	GetWorld()->OverlapMultiByChannel(Overlaps, CurrentLoc, FQuat::Identity,
		ECC_Pawn, FCollisionShape::MakeSphere(BounceSearchRadius), QueryParams);

	AActor* NextTarget = nullptr;
	USceneComponent* NextTargetComp = nullptr;
	float MinDist = FLT_MAX;

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Candidate = Overlap.GetActor();
		if (!Candidate || Candidate == OtherActor)
		{
			continue;
		}

		UAbilitySystemComponent* CandidateASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Candidate);
		if (!CandidateASC || !CandidateASC->HasMatchingGameplayTag(RSTags::Team_Enemy))
		{
			continue;
		}

		// MoveIgnoreActors에 등록된 액터(이미 맞은 적) 제외
		if (SphereComp->GetMoveIgnoreActors().Contains(Candidate))
		{
			continue;
		}

		const float Dist = FVector::Dist(CurrentLoc, Candidate->GetActorLocation());
		if (Dist < MinDist)
		{
			MinDist = Dist;
			NextTarget = Candidate;
			NextTargetComp = Candidate->GetRootComponent();
		}
	}

	if (NextTarget && NextTargetComp)
	{
		// 다음 타겟으로 유도 전환
		ProjectileComp->bIsHomingProjectile = true;
		ProjectileComp->HomingTargetComponent = NextTargetComp;
		ProjectileComp->HomingAccelerationMagnitude = InitData.TurnSpeed > 0.f ? InitData.TurnSpeed : 3000.f;

		KHS_INFO(TEXT("Bounce %d → %s"), BounceHitCount, *NextTarget->GetName());
	}
	else
	{
		// 다음 타겟 없음 — 직선 비행 후 수명 만료 대기
		ProjectileComp->bIsHomingProjectile = false;
		KHS_INFO(TEXT("Bounce %d — 다음 타겟 없음, 직선 비행"), BounceHitCount);
	}
}

void ABaseProjectile::OnLifetimeExpired()
{
	OnProjectileExpired();
	GET_WORLD_SUBSYSTEM(UPoolingSubsystem, PoolSys);
	PoolSys->ReturnToPool(this);
}

