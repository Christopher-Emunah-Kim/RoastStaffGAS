# PLAN_HOMING_ARC_v1.0

> 작성일: 2026-03-19
> 관련 기획서: 스킬 시스템 기획 v1.4.md, PLAN_SPRINT1_v1.3.md
> 스프린트: SPRINT 1
> **선행 조건**: PLAN_POOLING_v1.0 구현 완료 후 진행

## 구현 목표

이동 타입 `EMoveType::HOMING`(유도형)과 `EMoveType::ARC`(곡사형) 투사체를 동시에 구현한다.

| 타입 | 동작 | 전용 파라미터 테이블 |
|------|------|----------------------|
| HOMING | `LockRange` 내 최근접 적을 `TurnSpeed`로 회전 추적 | Skill_Attack_MoveType_Params_Homing |
| ARC | `LaunchAngle`로 발사 후 `GravityScale` 포물선 비행, 착탄 시 `HitRadius` 범위 폭발 | Skill_Attack_MoveType_Params_Arc |

두 타입 모두 `PLAN_POOLING_v1.0`의 풀링 시스템과 통합한다.

---

## 영향 범위

### 수정/생성이 필요한 C++ 클래스

| 구분 | 클래스 | 파일 위치 |
|------|--------|-----------|
| 신규 | UGA_HomingAttack | Public/GAS/Abilities/Attack/GA_HomingAttack.h/.cpp |
| 신규 | AHomingProjectile | Public/Objects/Projectile/HomingProjectile.h/.cpp |
| 신규 | UGA_ArcAttack | Public/GAS/Abilities/Attack/GA_ArcAttack.h/.cpp |
| 신규 | AArcProjectile | Public/Objects/Projectile/ArcProjectile.h/.cpp |

> `UGA_HomingAttack`, `UGA_ArcAttack` 모두 **`UGA_ProjectileAttack`을 상속**하여 공통 로직(파라미터 조회, 스폰 패턴)을 재사용. 정확한 상속 구조는 `/code` 단계에서 `GA_ProjectileAttack` 코드 확인 후 확정.

### 수정이 필요한 기존 파일

| 파일 | 변경 내용 |
|------|-----------|
| Data/DataTableStructs.h | `FSkillAttackMoveTypeParamsHoming`, `FSkillAttackMoveTypeParamsArc` 신규 구조체 추가 |
| Data/RuntimeDataStructs.h | `FHomingInitData`, `FArcInitData` 추가 또는 `FProjectileInitData`에 Union 확장 |

### 수정/생성이 필요한 DataTable

| 테이블 | 변경 내용 |
|--------|-----------|
| Skill_Attack_MoveType_Params_Homing | 신규 생성 (SkillEffectID, TurnSpeed, LockRange) |
| Skill_Attack_MoveType_Params_Arc | 신규 생성 (SkillEffectID, LaunchAngle, GravityScale, HitRadius) |
| DT_Weapon_Static_Data | HOMING·ARC 무기 행 추가 (테스트용) |
| DT_Skill 관련 테이블 | HOMING·ARC 스킬 행 추가 (테스트용) |

### 추가 필요한 Gameplay Tag

| Tag | 용도 |
|-----|------|
| `Ability.Skill.Attack.Homing` | GA_HomingAttack 식별 |
| `Ability.Skill.Attack.Arc` | GA_ArcAttack 식별 |

---

## 함수 호출 흐름

### Q. HOMING — 유도형 발동 흐름

```
EquipmentSubsystem::FireSlot → SendGameplayEventToActor
    → UGA_HomingAttack::OnAbilityActivated()
        │
        ▼
    GDS에서 Homing 파라미터 조회
    (TurnSpeed, LockRange — Skill_Attack_MoveType_Params_Homing)
        │
        ▼
    GA 내부: LockRange 반경 OverlapMultiByChannel → Team_Enemy 필터
        ├─ 적 발견 → 최근접 적의 RootComponent(WeakPtr) 획득
        └─ 적 없음 → HomingTarget = nullptr (직선 비행 폴백. KHS_WARN)
        │
        ▼
    PoolingSubsystem->SpawnPooledActor<AHomingProjectile>(Class, SpawnTransform)
    → Actor->InitProjectile(FHomingInitData)
        → ProjectileComp->bIsHomingProjectile = true
        → ProjectileComp->HomingTargetComponent = TargetWeakPtr
        → ProjectileComp->HomingAccelerationMagnitude = TurnSpeed
        → SetLifeSpan(Lifetime)
        │
        ▼
    [비행] ProjectileMovementComponent 매 프레임 방향 보정
        │
        ▼
    OnProjectileHit() — Single HitType
        → ApplyEffectToTarget()
        → PoolingSubsystem->ReturnToPool(this)
```

### Q. HOMING — 타겟 소멸 시 어떻게 처리되는가?

`HomingTargetComponent`를 `TWeakObjectPtr`로 보유 → 타겟 사망 시 자동 null.
`ProjectileMovementComponent`는 null HomingTarget을 직선 비행으로 자동 처리 (엔진 기본 동작).

---

### Q. ARC — 곡사형 발동 흐름

```
EquipmentSubsystem::FireSlot → SendGameplayEventToActor
    → UGA_ArcAttack::OnAbilityActivated()
        │
        ▼
    GDS에서 Arc 파라미터 조회
    (LaunchAngle, GravityScale, HitRadius — Skill_Attack_MoveType_Params_Arc)
        │
        ▼
    SpawnTransform: 캐릭터 루트 기준 에임 방향 2m 앞 (발사형 기본 규칙)
        │
        ▼
    PoolingSubsystem->SpawnPooledActor<AArcProjectile>(Class, SpawnTransform)
    → Actor->InitProjectile(FArcInitData)
        → LaunchAngle로 초기 속도 벡터 계산 (에임 방향 + 수직 성분)
        → ProjectileComp->GravityScale = GravityScale
        → HitRadius 캐싱
        → SetLifeSpan(Lifetime)
        │
        ▼
    [비행] 포물선 궤적 — ProjectileMovementComponent + GravityScale
        │
        ▼
    OnProjectileHit() or OnLifetimeExpired() — Area HitType
        → 착탄 위치 기준 HitRadius 내 OverlapMultiByChannel → Team_Enemy 필터
        → 거리 기반 데미지 감쇄 적용 (기획서 Area 판정 규칙)
            • 30% 미만: 100%
            • 30~70%: 70%
            • 70% 이상: 40%
        → 범위 내 각 적에게 ApplyEffectToTarget()
        → PoolingSubsystem->ReturnToPool(this)
```

### Q. ARC의 HitRadius는 Arc 파라미터와 Area 파라미터 중 어느 것을 사용하는가?

**`Skill_Attack_HitType_Params_Area`의 `HitRadius`를 사용한다.**
데미지 감쇄 배율(30%/70% 구간 규칙)은 기획서에 하드 명시된 구간 공식을 직접 적용.
`Skill_Attack_MoveType_Params_Arc`의 HitRadius 필드는 움직임 로직과 관계없으므로 삭제한다.

> 이 결정은 두 테이블의 HitRadius 중복을 방지한다.

### Q. ARC 수명 만료(미착탄) 시 어떻게 처리되는가?

착탄과 동일한 Area 폭발 처리를 수행한 후 `ReturnToPool()`한다.

---

## 예외처리 목록

### HOMING

| 상황 | 처리 방식 |
|------|-----------|
| LockRange 내 적 없음 | 직선 비행 폴백. `KHS_WARN` 로그 |
| 추적 중 타겟 사망 | `TWeakObjectPtr` → null 자동 처리 → 직선 비행 전환 |
| Homing 파라미터 테이블 조회 실패 | `KHS_WARN` 후 `EndAbility`. 슬롯 유지 |
| 수명 만료 | `OnLifetimeExpired()` → `ReturnToPool()` |

### ARC

| 상황 | 처리 방식 |
|------|-----------|
| 착탄 범위 내 적 없음 | 폭발 처리만 수행 후 `ReturnToPool()`. 정상 흐름 |
| Arc 파라미터 테이블 조회 실패 | `KHS_WARN` 후 `EndAbility`. 슬롯 유지 |
| `LaunchAngle` 계산 결과 벡터 Zero | `KHS_WARN` 후 기본 방향(에임 방향) 사용 |
| 수명 만료 (미착탄) | `OnLifetimeExpired()` → 착탄과 동일한 Area 폭발 처리 → `ReturnToPool()` |

---

## [검토 결과]

- **기획서 일관성**: 스킬 시스템 기획 v1.4 HOMING·ARC 명세와 일치.
- **누락된 예외처리**: 없음.
- **기획서 미정의 항목**:
  - `GA_ProjectileAttack` 상속 구조 확정 여부 — `/code` 단계에서 기존 코드 확인 후 결정.
  - `FProjectileInitData` Union 확장 vs 별도 파생 구조체 — `/code` 단계 확정.
- **기획서 정정 필요 사항**:
  - PLAN_SPRINT1_v1.3 미구현 항목 목록에 HOMING·ARC 추가 필요.
