# PLAN_POOLING_v1.0

> 작성일: 2026-03-19
> 관련 기획서: 스킬 시스템 기획 v1.4.md, HANDOFF_2026-03-19.md
> 스프린트: SPRINT 1

## 구현 목표

인게임 모든 풀링 가능 오브젝트(소환 오브젝트·투사체)의 반복 Spawn/Destroy 부하를 방지하는 **URSPoolingSubsystem**을 구현한다.
향후 에너미 스폰·데미지 플로팅 위젯 등으로의 확장을 고려하여, **Actor Pool / Widget Pool** 두 채널을 내부에 분리하여 설계한다.

---

## 영향 범위

### 수정/생성이 필요한 C++ 클래스

| 구분 | 클래스 / 인터페이스 | 파일 위치 |
|------|---------------------|-----------|
| 신규 | IRSPoolableInterface | Public/Interfaces/RSPoolableInterface.h |
| 신규 | URSPoolingSubsystem | Public/Subsystems/RSPoolingSubsystem.h/.cpp |
| 수정 | ABaseSummonObject | Public/Objects/Summon/BaseSummonObject.h/.cpp |
| 수정 | ABaseProjectile | Public/Objects/Projectile/BaseProjectile.h/.cpp |

### 수정/생성이 필요한 DataTable / Gameplay Tag

없음.

### 에디터 설정(BP/에셋) 필요 여부

없음 (순수 C++ 레이어).
`InitializePool` 호출 위치는 **기획서 미정의 항목** — 현 스프린트에서는 `GameMode::BeginPlay` 임시 사용.

---

## 확장성 설계 방침

| 채널 | 대상 타입 | 현 스프린트 | 이후 스프린트 |
|------|-----------|-------------|----------------|
| Actor Pool | `AActor` 파생 전체 | SummonObject, Projectile **완전 구현** | EnemyCharacter 등 |
| Widget Pool | `UUserWidget` 파생 전체 | **API 스켈레톤만 선언** | 데미지 플로팅 위젯 등 |

> **이유**: `UUserWidget`은 `CreateWidget`/`RemoveFromParent` 수명 주기를 가지므로 AActor와 별도 채널 필요. `IRSPoolableInterface`는 `UObject` 수준에서 정의하여 양쪽 모두 구현 가능.

---

## 인터페이스 설계

```cpp
// RSPoolableInterface.h
UINTERFACE(MinimalAPI, BlueprintType)
class URSPoolableInterface : public UInterface { GENERATED_BODY() };

class IRSPoolableInterface
{
    GENERATED_BODY()
public:
    // 풀에서 꺼낼 때 — 활성화, 초기 데이터 세팅 후 이 시점에 GE/이동 시작
    virtual void OnPoolActivate() = 0;
    // 풀에 반납할 때 — 비활성화, 타이머/상태 초기화
    virtual void OnPoolDeactivate() = 0;
};
```

---

## 서브시스템 데이터 구조

```cpp
// RSPoolingSubsystem.h (핵심 멤버)

// ── Actor Pool ──────────────────────────────
TMap<UClass*, TArray<TObjectPtr<AActor>>> ActorPool;

UPROPERTY()   // ← GC 강참조 필수 (HANDOFF 명시 버그 방지)
TSet<TObjectPtr<AActor>> ActiveActors;

// ── Widget Pool (스켈레톤 — 현 스프린트 미구현) ──
TMap<UClass*, TArray<TObjectPtr<UUserWidget>>> WidgetPool;

UPROPERTY()
TSet<TObjectPtr<UUserWidget>> ActiveWidgets;
```

---

## 함수 호출 흐름

### Q. 스테이지 시작 시 풀은 어떻게 예열되는가?

```
GameMode::BeginPlay (임시)
    → URSPoolingSubsystem::InitializePool(SummonClass, WarmupCount)
    → URSPoolingSubsystem::InitializePool(ProjectileClass, WarmupCount)
        각각: SpawnActor × Count → OnPoolDeactivate() → ActorPool[Class].Push
```

### Q. 소환 오브젝트 스폰/반환 흐름 (변경 전후)

```
[기존] GA → SpawnActorDeferred → InitSummon → FinishSpawning → BeginPlay (GE+LifeSpan)
[변경]
GA_SummonBase::SpawnSummonObject()
    → PoolingSubsystem->SpawnPooledActor<ABaseSummonObject>(Class, Transform)
        ├─ 풀에 있음 → SetActorTransform → return Actor
        └─ 풀 비어있음 (KHS_WARN) → SpawnActor → return Actor
    → Actor->InitSummon(InitData)       ← GA가 InitData 전달
        → bInitialized = true

ABaseSummonObject::OnPoolActivate()     ← SpawnPooledActor 내부에서 호출
    → SetActorHiddenInGame(false) / 충돌 활성화
    → check(bInitialized)               ← InitSummon 전 호출 방지
    → ApplyGameplayEffectToArea()
    → SetLifeSpan(InitData.Lifetime)

LifeSpanExpired()
    → PoolingSubsystem->ReturnToPool(this)
        → OnPoolDeactivate()
            → SetActorHiddenInGame(true) / 충돌 비활성화
            → SetLifeSpan(0.f)
            → bInitialized = false
        → ActiveActors에서 제거 → ActorPool[Class].Push
```

> **설계 결정**: `InitSummon → OnPoolActivate` 호출 순서는 `/code` 단계에서 GA 코드와 함께 확정.

### Q. 투사체 스폰/반환 흐름

```
GA_ProjectileAttack (또는 GA_HomingAttack, GA_ArcAttack)
    → PoolingSubsystem->SpawnPooledActor<ABaseProjectile>(Class, Transform)
    → Actor->InitProjectile(InitData)
        → BeginPlay에 있던 초기화 로직을 InitProjectile로 이전 (풀 재사용 시 BeginPlay 미호출)
        → 속도/GE 클래스/Lifetime 세팅

ABaseProjectile::OnPoolActivate()
    → SetActorHiddenInGame(false) / 충돌 활성화

OnHit() or OnLifetimeExpired()
    → [기존] Destroy()
    → [변경] PoolingSubsystem->ReturnToPool(this)
        → OnPoolDeactivate()
            → ProjectileComp 정지 / 속도 초기화
            → SetActorHiddenInGame(true) / 충돌 비활성화
            → LifetimeTimer 클리어
        → ActorPool[Class].Push
```

---

## 예외처리 목록

| 상황 | 처리 방식 |
|------|-----------|
| `InitializePool` 없이 `SpawnPooledActor` 호출 | 신규 SpawnActor 처리. `KHS_WARN` 출력 |
| `ReturnToPool` 시 이미 풀에 존재하는 액터 | `ActiveActors.Contains` 체크 → 중복 Push 방지. `KHS_WARN` 후 무시 |
| `SpawnPooledActor` 시 Class == null | `KHS_WARN` 후 `nullptr` 반환 |
| `ActiveActors` `UPROPERTY()` 누락 | GC null 포인터 위험 — `UPROPERTY()` 강참조 필수 |
| `OnPoolActivate` 시 `bInitialized == false` | `KHS_WARN` 후 조기 반환 |
| `ReturnToPool` 시 Actor가 `ActiveActors`에 없음 | `KHS_WARN` 후 무시 (이중 반납 방어) |

---

## [검토 결과]

- **기획서 일관성**: 스킬 시스템 기획 v1.4, HANDOFF_2026-03-19와 일치.
- **누락된 예외처리**: 없음.
- **기획서 미정의 항목**:
  - `InitializePool` 호출 위치 (스테이지 기획 미확정 → 임시 BeginPlay)
  - `ReturnAllActiveActors` 호출 시점 (스테이지 리셋 기획 미확정 → 선언만 포함)
  - Widget Pool 구현 시점 (현 스프린트 외)
- **기획서 정정 필요 사항**: 없음.
