# PLAN_PIERCE_HitType_v1.0

> 작성일: 2026-03-25
> 관련 기획서: 스킬 시스템 기획 v1.4.md
> 스프린트: SPRINT 4

## 구현 목표

`EHitType::PIERCE` 관통형 투사체 구현.
최대 `PierceCount`회까지 순차 관통하며 데미지 부여. 관통마다 `DamageDecay` 비율로 감쇠.
중복 타격 방지는 `SphereComp->IgnoreActorWhenMoving`으로 엔진 레벨에서 처리.

**신규 파일 없음.**

---

## 영향 범위

### 수정 파일

| 파일 | 변경 내용 |
|------|-----------|
| `RuntimeDataStructs.h` | `FProjectileInitData`에 `PierceCount(int32)`, `DamageDecay(float)` 추가 |
| `BaseProjectile.h` | `bHasPierceFinished(bool)` 멤버 추가, `HandlePierceHit` private 선언 |
| `BaseProjectile.cpp` | `OnHit` PIERCE 분기, `HandlePierceHit` 구현, `OnPoolDeactivate`/`InitProjectile` 리셋 |
| `GA_ProjectileAttack.h` | `HandlePierceType` private 선언 |
| `GA_ProjectileAttack.cpp` | `HandleExtraParametersByType` PIERCE 분기, `HandlePierceType` 구현 |

### DataTable (에디터 작업)

| 테이블 | 작업 |
|--------|------|
| `DT_Skill_Attack_Common_Params` | SKILL_THUNDER 행 HitType → `PIERCE` 변경 |
| `DT_Skill_Attack_HitType_Param_Pierce` | SKILL_THUNDER의 SkillEffectID 행 추가: PierceCount=4, DamageDecay=0.2 |

---

## 멤버 변수 (BaseProjectile.h private 섹션)

```cpp
// HitType - Pierce
bool bHasPierceFinished = false;   // ReturnToPool 이중 호출 가드
// 중복 방지: SphereComp->IgnoreActorWhenMoving (엔진 레벨 관리)
// 관통 횟수: SphereComp->CopyArrayOfMoveIgnoreActors().Num()
```

---

## 함수 호출 흐름

### Phase A — GA 파라미터 수집

```
HandleExtraParametersByType:
  if HitType == PIERCE:
    HandlePierceType(OutInitData, ExecData)

HandlePierceType:
  GDS->GetHitTypeData<FSkillAttackHitTypeParamsPierce>(SkillEffectID, PierceData)
    실패 → KHS_WARN + return false
  OutInitData.PierceCount = PierceData.PierceCount
  OutInitData.DamageDecay = PierceData.DamageDecay
  return true
```

### Phase B — 투사체 충돌 분기

```
OnHit:
  ① HitType == AREA   → ExplodeArea + ReturnToPool (기존)
  ② HitType == PIERCE → HandlePierceHit(OtherActor)  ← NEW
  ③ else (SINGLE)     → HandleHitEvent(OtherActor, Hit) (기존)
```

### Phase C — `HandlePierceHit` (private)

```
HandlePierceHit(AActor* OtherActor):
  1. if bHasPierceFinished → return  // ReturnToPool 이중 호출 가드
  2. TargetASC = GetAbilitySystemComponent(OtherActor)
     if !TargetASC → return   (벽/지형 — ignore 추가 없이 통과)
  3. if !HasMatchingGameplayTag(Team_Enemy) → return
  4. SphereComp->IgnoreActorWhenMoving(OtherActor, true)
     // 이후 이 적과의 OnHit는 물리 레벨에서 차단 → 중복 타격 방지
  5. const int32 HitCount = SphereComp->CopyArrayOfMoveIgnoreActors().Num()
  6. const float Multiplier = FMath::Max(0.f, 1.f - (HitCount - 1) * InitData.DamageDecay)
     // HitCount=1: 1.0 / 2: 0.8 / 3: 0.6 / 4: 0.4
  7. ApplyMultipleEffectsToTarget(TargetASC, Multiplier)
  8. if HitCount >= InitData.PierceCount:
       bHasPierceFinished = true
       ReturnToPool()
```

### Phase D — 상태 리셋

```
OnPoolDeactivate (기존 리셋 블록 아래에 추가):
  SphereComp->ClearMoveIgnoreActors()
  bHasPierceFinished = false

InitProjectile (bHasExploded = false 옆에 추가):
  SphereComp->ClearMoveIgnoreActors()
  bHasPierceFinished = false
```

---

## 예외처리 목록

| # | 상황 | 처리 |
|---|------|------|
| 1 | GDS Pierce 파라미터 조회 실패 | KHS_WARN + return false → EndAbility |
| 2 | PierceCount ≤ 0 | 첫 타에 즉시 ReturnToPool (`HitCount >= PierceCount`) |
| 3 | DamageDecay > 1.0 | `FMath::Max(0.f, ...)` 클램프로 음수 multiplier 방지 |
| 4 | 동일 적 재충돌 | `IgnoreActorWhenMoving` 등록으로 물리 레벨에서 OnHit 자체 차단 |
| 5 | 벽/지형 충돌 | TargetASC 없음 → IgnoreActorWhenMoving 등록 없이 return (카운트 증가 없음) |
| 6 | 마지막 타격 직후 수명 만료 | `bHasPierceFinished` 가드로 ReturnToPool 이중 호출 차단 |
| 7 | 수명 만료 시 소멸 | `OnLifetimeExpired → ReturnToPool` 기존 동작 유지 |

---

## [검토 결과]

**기획서 일관성**: 스킬 시스템 기획 v1.4 §타격 타입/§DataTable 스키마와 일치.

**Gemini 리뷰 반영**:

| 항목 | 처리 |
|------|------|
| UPROPERTY 누락 우려 | `IgnoreActorWhenMoving` 채택으로 TSet 멤버 자체 제거 |
| ReturnToPool 이중 호출 | `bHasPierceFinished` 가드 반영 |
| FSkillAttackHitTypeParamsPierce 선언 누락 | DataTableStructs.h:286에 이미 존재 — false positive |
| StatusGE 적용 기준 | 모든 관통 대상 적용(A안) 사용자 확정 |

---

## 구현 결과 (CODE 단계 완료 후 기입)

- [ ] RuntimeDataStructs.h 필드 추가
- [ ] BaseProjectile.h/cpp 구현
- [ ] GA_ProjectileAttack.h/cpp 구현
- [ ] DataTable 에디터 작업
