# PLAN_HOMING_ARC_NoNewFiles_v1.0

> 작성일: 2026-03-24
> 기반: PLAN_HOMING_ARC_v1.0 설계 결정 유지, 신규 파일 없이 기존 2클래스에 통합
> 관련 기획서: 스킬 시스템 기획 v1.4.md
> 스프린트: SPRINT 1
> **선행 조건**: PLAN_POOLING_v1.0 구현 완료 (완료됨)

---

## 구현 목표

`EMoveType::HOMING`(유도형) / `EMoveType::ARC`(곡사형)을 **신규 파일 없이**
기존 `ABaseProjectile` + `UGA_ProjectileAttack`에 MoveType/HitType 분기로 흡수한다.

---

## 아키텍처 결정 메모

**왜 자식 클래스 분리 대신 베이스 통합을 선택하는가?**

현재 투사체 타입 수(LINEAR·HOMING·ARC·PIERCE, 4개)와 기획서 내 조합 미정의 상태에서는
베이스 통합이 오버엔지니어링 없이 적합하다.
타입이 5개를 초과하거나 HOMING+PIERCE 같은 교차 조합이 생기면 자식 분리로 리팩토링.

**FProjectileInitData에 모든 타입 파라미터를 두는 것이 적합한가?**

구조체는 로직 없는 데이터 묶음이므로 SRP 엄격 적용 대상 아님.
GA → 투사체 1회 전달용 파라미터 팩으로, 타입별로 쓰는 필드가 달라도 메모리 영향 없음.
필드 40개 이상 증가 시점에 타입별 구조체 분리 검토.

---

## 수정 파일 목록 (신규 파일 없음)

| 파일 | 변경 요약 |
|------|-----------|
| `Data/RuntimeDataStructs.h` | `FProjectileInitData`에 MoveType·HitType·Homing·Arc·Area 필드 추가 |
| `Objects/Projectile/BaseProjectile.h` | `HomingTarget` + `bHasExploded` 멤버 추가; `SetHomingTarget` 세터; `ExplodeArea` 선언; `OnProjectileInitialized`/`OnProjectileExpired` 인라인 빈구현 제거 |
| `Objects/Projectile/BaseProjectile.cpp` | `OnProjectileInitialized`(HOMING/ARC 분기), `OnHit`(AREA 분기+Guard), `OnProjectileExpired`(AREA Guard+폭발), `ExplodeArea` 구현; `InitProjectile` 리셋 추가 |
| `GAS/Abilities/GA_ProjectileAttack.cpp` | `BuildInitData` MoveType/HitType 복사 추가; `PrepareProjectileData` Homing·Arc·Area 파라미터 조회 및 타겟 탐색 추가 |

---

## Step 1 — `FProjectileInitData` 확장

```cpp
// 이동/타격 타입
UPROPERTY() EMoveType MoveType = EMoveType::LINEAR;
UPROPERTY() EHitType  HitType  = EHitType::SINGLE;

// HOMING — TurnSpeed는 UPM HomingAccelerationMagnitude에 직접 대입 (단위: cm/s²)
UPROPERTY() float TurnSpeed = 0.f;

// ARC — LaunchAngle 유효 범위 -80~80도 (PrepareProjectileData에서 Clamp 처리)
UPROPERTY() float LaunchAngle  = 0.f;
UPROPERTY() float GravityScale = 0.f;

// AREA
UPROPERTY() float HitRadius = 0.f;

// ※ HomingTarget 제외 — ABaseProjectile protected 멤버로 관리 (USTRUCT non-UPROPERTY 포인터 위험 회피)
```

---

## Step 2 — `ABaseProjectile` 헤더 추가

```cpp
public:
    // GA에서 HOMING 타겟 주입
    void SetHomingTarget(USceneComponent* Target) { HomingTarget = Target; }

protected:
    // HOMING 추적 타겟 (TWeakObjectPtr — 타겟 사망 시 자동 null)
    TWeakObjectPtr<USceneComponent> HomingTarget;

private:
    // AREA 이중 폭발 방지
    bool bHasExploded = false;

    // AREA 폭발 처리 헬퍼
    void ExplodeArea(const FVector& Center);
```

`OnProjectileInitialized()`, `OnProjectileExpired()` — 인라인 `{ }` 제거, cpp에서 구현.

---

## Step 3 — `GA_ProjectileAttack.cpp` 수정

### `BuildInitData` 추가 (2줄)
```cpp
OutInitData.MoveType = ExecData.MoveType;
OutInitData.HitType  = ExecData.HitType;
```

### `PrepareProjectileData` — `BuildInitData` 호출 후 추가

```
// ─── HOMING ───
if MoveType == HOMING:
    GDS->GetMoveTypeData<FSkillAttackMoveTypeParamsHoming>(SkillEffectID, HomingData)
        실패 → KHS_WARN + return false
    OutInitData.TurnSpeed = HomingData.TurnSpeed

    OverlapMultiByChannel(CasterLocation, ECC_Pawn, Sphere(HomingData.LockRange))
        + AddIgnoredActor(CachedInstigator)
    Team_Enemy 필터 → DistSquared 최소값 기준 최근접 액터 선택
        있음 → (스폰 후) Projectile->SetHomingTarget(NearestEnemy->GetRootComponent())
        없음 → KHS_WARN("LockRange 내 적 없음 — 직선 비행 폴백")

// ─── ARC ───
else if MoveType == ARC:
    GDS->GetMoveTypeData<FSkillAttackMoveTypeParamsArc>(SkillEffectID, ArcData)
        실패 → KHS_WARN + return false
    OutInitData.LaunchAngle  = FMath::Clamp(ArcData.LaunchAngle, -80.f, 80.f)
    OutInitData.GravityScale = ArcData.GravityScale

// ─── AREA (MoveType과 독립 — ARC+AREA 조합 지원) ───
if HitType == AREA:
    GDS->GetHitTypeData<FSkillAttackHitTypeParamsArea>(SkillEffectID, AreaData)
        실패 → KHS_WARN + return false
    OutInitData.HitRadius = AreaData.HitRadius
```

> `SetHomingTarget` 호출 시점: `SpawnProjectiles` 반환값 또는 오버로드로 스폰된 투사체 포인터 획득 후 호출.
> SpawnProjectiles가 포인터를 반환하지 않을 경우, `/coding` 단계에서 확인 후 방식 결정.

---

## Step 4 — `BaseProjectile.cpp` 구현

### `InitProjectile` — 리셋 추가 (풀 재사용 대비)
```cpp
bHasExploded = false;
HomingTarget.Reset();
// 이하 기존 로직 유지
```

### `OnProjectileInitialized`

```
HOMING:
    HomingTarget.IsValid()
        ✓ → ProjectileComp->bIsHomingProjectile = true
              ProjectileComp->HomingTargetComponent = HomingTarget
              ProjectileComp->HomingAccelerationMagnitude = InitData.TurnSpeed
        ✗ → KHS_WARN("HOMING 타겟 없음 — 직선 비행 폴백. SkillID: %s")

ARC:
    RightVec = Cross(GetActorForwardVector(), FVector::UpVector).GetSafeNormal()
    LaunchVel = GetActorForwardVector().RotateAngleAxis(InitData.LaunchAngle, RightVec) * InitData.Speed
    LaunchVel.IsNearlyZero()
        ✓ → KHS_WARN("LaunchAngle 계산 Zero — 에임 방향 폴백. SkillID: %s")
        ✗ → ProjectileComp->Velocity = LaunchVel
    ProjectileComp->ProjectileGravityScale = InitData.GravityScale
```

### `OnHit` 수정 — Owner 체크 직후 AREA 분기 삽입

```
if HitType == AREA:
    if bHasExploded → return
    bHasExploded = true
    ExplodeArea(Hit.ImpactPoint)    // 벽/지형 충돌에도 폭발 (기획서 "착탄 시 폭발" 규칙)
    PoolSys->ReturnToPool(this)
    return

// 이하 기존 SINGLE 로직 유지
```

### `ExplodeArea(const FVector& Center)`

```
OverlapMultiByChannel(Center, ECC_Pawn, Sphere(HitRadius))
    + AddIgnoredActor(this, GetInstigator())

각 Overlap:
    TargetASC 없음 or !Team_Enemy → skip
    Ratio = Dist(Center, EnemyLocation) / HitRadius  // 0 ~ 1.0
    Ratio ≤ 0.3 → Multiplier = 1.0f   (100%)
    Ratio ≤ 0.7 → Multiplier = 0.7f   (70%)
    Ratio > 0.7 → Multiplier = 0.4f   (40%)
    ApplyEffectToTarget(TargetASC, DamageGEClass, Amount * Multiplier)
    if StatusGEClass: ApplyEffectToTarget(TargetASC, StatusGEClass, 0.f)
```

### `OnProjectileExpired`

```
if HitType == AREA:
    if bHasExploded → return
    bHasExploded = true
    ExplodeArea(GetActorLocation())
// ReturnToPool은 OnLifetimeExpired()가 담당
```

---

## 예외처리

| 상황 | 처리 |
|------|------|
| Homing/Arc/Area 파라미터 조회 실패 | KHS_WARN + return false → EndAbility. 슬롯 유지 |
| LockRange 내 적 없음 | KHS_WARN + HomingTarget null → 직선 비행 폴백 |
| 추적 중 타겟 사망 | TWeakObjectPtr null → UPM 직선 비행 전환 |
| LaunchAngle 범위 초과 (-80~80) | FMath::Clamp로 자동 보정 |
| LaunchAngle 계산 Zero 벡터 | KHS_WARN + Velocity 미교체 (에임 방향 유지) |
| ARC 수명 만료 (미착탄) | OnProjectileExpired → ExplodeArea → OnLifetimeExpired → ReturnToPool |
| OnHit·Expired 이중 폭발 경로 | bHasExploded Guard로 두 번째 호출 차단 |
| HitRadius = 0 (DataTable 설정 오류) | OverlapMulti 반경 0 → 적 0 → ReturnToPool |
| 벽/지형 충돌 (AREA) | 의도된 동작 — 착탄 대상 무관 폭발 |

---

## 함수 호출 흐름

### HOMING

```
GA_ProjectileAttack::PrepareProjectileData
  ├─ GetMoveTypeData<Homing> → TurnSpeed, LockRange
  ├─ OverlapMulti(LockRange) → NearestEnemy
  └─ SpawnProjectiles → Projectile->SetHomingTarget(RootComp)
                      → InitProjectile(InitData)
                          └─ OnProjectileInitialized()
                               └─ UPM bIsHomingProjectile = true (매 틱 방향 보정)

OnHit(Enemy) → SINGLE path → ApplyEffect → ReturnToPool
타겟 소멸    → HomingTarget WeakPtr null → UPM 직선 비행 전환
```

### ARC

```
GA_ProjectileAttack::PrepareProjectileData
  ├─ GetMoveTypeData<Arc>  → LaunchAngle, GravityScale
  ├─ GetHitTypeData<Area>  → HitRadius
  └─ SpawnProjectiles → InitProjectile(InitData)
                          └─ OnProjectileInitialized()
                               ├─ Velocity = RotateAngleAxis(LaunchAngle) * Speed
                               └─ ProjectileGravityScale = GravityScale

OnHit(any)  → AREA path → ExplodeArea(ImpactPoint) → ReturnToPool
수명만료    → OnProjectileExpired → ExplodeArea(Location) → OnLifetimeExpired → ReturnToPool
```

---

## 이후 리팩토링 할일 (구현 완료 후)

> **OnHit/OnProjectileInitialized/PrepareProjectileData에 대한 내부로직을 private헬퍼로 추출**
> **클래스 분리 등은 현재 구현 범위 아님. 타입이 5개 이상으로 늘어나거나 OnHit/PrepareProjectileData가 비대해지면 진행.**
> 
| # | 대상 | 작업 내용 |
|---|------|-----------|
| 1 | `BaseProjectile::OnHit` | AREA 분기 로직 → `HandleAreaHit(const FHitResult&)` private 헬퍼 추출 |
| 2 | `BaseProjectile::OnProjectileInitialized` | HOMING 세팅 → `InitHoming()`, ARC 세팅 → `InitArc()` private 헬퍼 추출 |
| 3 | `GA_ProjectileAttack::PrepareProjectileData` | Homing 타겟 탐색 → `FindHomingTarget(float LockRange) → AActor*` 헬퍼 추출 |
| 4 | 타입 5개 초과 시 | 자식 클래스 분리 검토 (`AHomingProjectile`, `AArcProjectile` 등) |

---

## [검토 결과]

- **Gemini 크로스 리뷰 반영 완료** (2026-03-24)
  - HomingTarget → ABaseProjectile 멤버 이관 (USTRUCT non-UPROPERTY 포인터 위험 회피)
  - ARC Velocity → RotateAngleAxis(RightVec) 방식으로 짐벌락 방지
  - bHasExploded Guard 추가 (이중 폭발 방지)
  - LaunchAngle Clamp(-80~80) 추가
- **기획서 일관성**: 스킬 시스템 기획 v1.4 HOMING·ARC 명세와 일치
- **누락된 예외처리**: 없음
- **기획서 미정의 항목**:
  - SpawnProjectiles의 투사체 포인터 반환 방식 — `/coding` 단계에서 확인 후 결정
  - ARC+HOMING 조합 — 기획서 미정의, 현재 지원 안 함 (단일 MoveType 분기)

---

## [구현 완료] 2026-03-24

### 계획서 대비 실제 구현 차이

| 항목 | 계획서 | 실제 구현 | 사유 |
|------|--------|-----------|------|
| HomingTarget 위치 | `ABaseProjectile` 멤버 + `SetHomingTarget()` | `FProjectileInitData.HomingTarget` (TWeakObjectPtr) | SpawnProjectiles 반환 시점에 InitProjectile이 이미 호출된 후라 SetHomingTarget 주입 불가 |
| GA 헬퍼 구조 | PrepareProjectileData 내 인라인 | `HandleHomingType` / `HandleArcType` / `HandleAreaType` / `FindNearestEnemy` 헬퍼 추출 | 사용자 요청으로 가독성 개선 |

### 구현 중 발견·수정된 버그

| 버그 | 파일 | 수정 내용 |
|------|------|-----------|
| `NearestEnemy` nullptr 역참조 | `GA_ProjectileAttack.cpp` | null 시 `return true`(폴백) 누락 → 추가 |
| `HandleExtraParametersByType` `bSuccess = false` 초기값 | `GA_ProjectileAttack.cpp` | LINEAR+SINGLE이 false 반환 → `true`로 수정 |
| `ExplodeArea` 동일 액터 중복 데미지 | `BaseProjectile.cpp` | `TSet<AActor*> HitActors` 로컬 변수로 중복 차단 |

### 테스트 결과

| TC | 타입 | 결과 | 비고 |
|----|------|------|------|
| HOMING 적 탐색 | HOMING | ✅ | LockRange=5000, TurnSpeed=500 기준 탐색 성공 |
| HOMING 오버슈팅 | HOMING | ⚠️ 미완 | TurnSpeed 부족 시 타겟 스쳐지나감. TurnSpeed≥3000 권장 (CSV 튜닝) |
| ARC 발사 | ARC | 미확인 | — |
| ARC 수명 폭발 | ARC | 미확인 | — |

> HOMING 오버슈팅은 코드 버그가 아닌 데이터 튜닝 사항. Speed 대비 TurnSpeed를 3~10배 수준으로 설정 필요.

### 시니어 리뷰 지적사항 및 처리 (SR_2026-03-24_HomingArcArea.md)

| 우선순위 | 항목 | 처리 결과 |
|---------|------|-----------|
| HIGH | `HomingTarget` UPROPERTY() 누락 | ✅ 수정 — `UPROPERTY()` 추가 |
| HIGH | `OnPoolDeactivate` HOMING/ARC 상태 리셋 누락 | ✅ 수정 — `bIsHomingProjectile`, `HomingTargetComponent`, `ProjectileGravityScale` 리셋 추가 |
| HIGH | `HandleExtraParametersByType` bSuccess 덮어쓰기 버그 | ✅ 수정 — 분기별 조기 `return false` 방식으로 변경 |
| MEDIUM | `ExplodeArea` 감쇠 경계값 `>` → `>=` | ✅ 수정 — 기획서 "70% 이상 → 40%" 규칙에 맞게 수정 |

### 기타 수정 (리뷰 외)

- `GA_SummonBase.cpp`: `AbilityTags.AddTag` deprecated 경고 → `SetAssetTags()` API로 교체