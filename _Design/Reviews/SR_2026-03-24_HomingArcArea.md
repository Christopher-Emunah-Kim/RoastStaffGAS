# [SENIOR REVIEW 결과] HOMING / ARC / AREA 투사체 타입 통합

> 리뷰 일자: 2026-03-24
> 리뷰 대상: FProjectileInitData 확장, BaseProjectile HOMING/ARC/AREA 분기, GA_ProjectileAttack 타입별 파라미터 조회
> 관련 계획서: PLAN_HOMING_ARC_NoNewFiles_v1.0.md
> 관련 기획서: 스킬 시스템 기획 v1.4.md

---

## 통과 항목

- Template Method 패턴 (InitProjectile -> OnProjectileInitialized, OnHit -> OnProjectileHit, OnLifetimeExpired -> OnProjectileExpired) 올바르게 활용
- bHasExploded Guard로 OnHit/OnProjectileExpired 이중 폭발 경로 차단 -- 계획서 명세와 일치
- TWeakObjectPtr<USceneComponent> HomingTarget -- 타겟 사망 시 자동 null, UPM 직선 비행 전환 안전
- FVector::DistSquared로 최근접 적 탐색 -- 불필요한 sqrt 회피 최적화
- LaunchAngle FMath::Clamp(-80, 80) 적용 -- 계획서 명세와 일치
- OverlapMultiByChannel + TSet<AActor*> 중복 방지 -- 기획서 "범위 내 모든 대상" 규칙 준수
- ARC 미착탄 수명 만료 시 ExplodeArea(GetActorLocation()) -- 기획서 소멸 조건 준수
- OnPoolDeactivate()에서 LifetimeTimerHandle ClearTimer -- 풀 재사용 시 이전 타이머 잔류 방지
- InitProjectile에서 bHasExploded = false 리셋 -- 풀 재사용 대비 상태 초기화
- HandleExtraParametersByType에서 MoveType/HitType 독립 분기 (ARC+AREA 조합 지원) -- 계획서 명세와 일치
- GAS 패턴: GameplayEvent 트리거, SetByCaller 태그 바인딩, InstigatorASC 컨텍스트 설정 모두 정석
- GA_Base의 ActivateAbility final + OnAbilityActivated 위임 구조 견고
- BuildInitData / HandleHomingType / HandleArcType / HandleAreaType 헬퍼 분리로 PrepareProjectileData 가독성 확보 -- 계획서의 리팩토링 항목 3번 선반영

---

## 수정 권고

| 파일 | 라인 | 관점 | 내용 | 심각도 |
|------|------|------|------|--------|
| RuntimeDataStructs.h | 183 | 메모리/GC | `TWeakObjectPtr<USceneComponent> HomingTarget`에 UPROPERTY() 매크로가 없다. USTRUCT 내 TWeakObjectPtr은 UPROPERTY()가 없으면 GC가 참조를 추적하지 않아 대상 GC 후 IsValid()가 정상 동작하지 않을 수 있다. 직렬화는 불필요하지만 GC 추적을 위해 UPROPERTY()를 붙여야 한다. | **HIGH** |
| BaseProjectile.cpp | 42-48 | 풀링/버그 | OnPoolDeactivate()에서 HOMING 상태를 리셋하지 않는다. `ProjectileComp->bIsHomingProjectile = true`로 설정된 투사체가 풀에 반환된 후 LINEAR 타입으로 재사용되면 여전히 Homing 모드가 켜져 있다. `bIsHomingProjectile = false`, `HomingTargetComponent = nullptr`, `ProjectileGravityScale = 0.f` 리셋 필요. | **HIGH** |
| BaseProjectile.cpp | 233 | 하드코딩 | ExplodeArea의 데미지 감쇠율 `0.7f`, `0.4f`와 구간 경계 `0.3f`, `0.7f`가 하드코딩되어 있다. 기획서 v1.4의 현재 규격과 일치하지만, CLAUDE.md "하드코딩 금지" 규칙 위반이다. DataTable(DT_Skill_Attack_HitType_Params_Area)에 감쇠 구간/배율 필드를 추가하거나, 최소한 FProjectileInitData에 감쇠 파라미터를 포함시켜야 한다. | **MEDIUM** |
| BaseProjectile.cpp | 233 | 기획서 정합성 | 기획서: "범위의 30% 미만 -> 100%, 30% 이상 70% 미만 -> 70%, 70% 이상 -> 40%". 코드: `Ratio > 0.7f -> 0.4f`, `Ratio > 0.3f -> 0.7f`, 그 외 1.0f. 경계값 동작이 다르다. 기획서는 "30% 미만"(exclusive)이므로 30% 정확히는 70% 구간, "70% 미만"(exclusive)이므로 70% 정확히는 40% 구간이어야 한다. 코드에서 `Ratio > 0.7f`는 70% 초과만 0.4f로 처리하므로, **Ratio == 0.7f일 때 0.7f 배율이 적용**되어 기획서("70% 이상은 40%")와 불일치한다. `>=` 연산자를 사용해야 한다. | **MEDIUM** |
| GA_ProjectileAttack.cpp | 104-127 | 버그/로직 | HandleExtraParametersByType에서 `bSuccess`가 이전 분기 결과를 덮어쓸 수 있다. MoveType이 HOMING이 아니고 ARC도 아닌 LINEAR인데 HitType이 AREA인 경우, HOMING/ARC 분기를 건너뛰어 bSuccess=true 유지 후 AREA 처리하므로 정상이다. 그러나 ARC+AREA 조합에서 ARC가 성공(bSuccess=true) 후 AREA가 실패하면 bSuccess=false로 올바르게 반환된다. 반대로 **ARC가 실패(bSuccess=false)해도 AREA 분기에 진입하여 bSuccess를 true로 덮어쓸 수 있다**. 이 경우 ARC 파라미터 없이 AREA만 설정된 불완전한 InitData가 생성된다. ARC 실패 시 early return하거나, 각 분기 결과를 AND 연산해야 한다. | **HIGH** |
| GA_ProjectileAttack.cpp | 158 | 성능/복사 | `FindNearestEnemy(FSkillAttackMoveTypeParamsHoming HomingData)` -- 구조체를 값으로 전달한다. `const FSkillAttackMoveTypeParamsHoming&`로 변경하여 불필요한 복사를 제거해야 한다. | **LOW** |
| GA_Base.h | 60 | 하드코딩 | `const float SPAWN_OFFSET = 200.f` -- 이전 리뷰(SR_2026-03-23)에서도 지적된 반복 패턴이다. DataTable 또는 EditDefaultsOnly로 외부화 필요. | **LOW** |
| GA_ProjectileAttack.h | 42 | 설계 | `FindNearestEnemy`가 GA_ProjectileAttack의 private 멤버로 구현되어 있다. 적 탐색은 HOMING 이외에도 향후 소환/유틸 스킬에서 재사용 가능성이 높다. GA_Base 또는 유틸 함수로 승격을 검토할 것. | **LOW** |

---

## 개선 제안

- **OnPoolDeactivate 리셋 체크리스트 패턴**: 새 MoveType/HitType 추가 시마다 OnPoolDeactivate에 리셋 항목을 빠뜨릴 위험이 있다. `InitProjectile` 시작부에서 `ProjectileComp` 전체를 기본값으로 리셋하는 `ResetProjectileState()` private 헬퍼를 도입하면, 풀 재사용 안전성이 구조적으로 보장된다.

- **ARC RightVec 계산의 엣지 케이스**: BaseProjectile.cpp L147에서 `FVector::CrossProduct(GetActorForwardVector(), FVector::UpVector)`는 투사체가 정확히 수직(Up/Down)을 향할 때 Zero 벡터가 된다. 쿼터뷰 게임에서 현실적으로 발생 확률이 매우 낮지만, GetSafeNormal() 결과가 Zero일 때의 폴백을 명시적으로 처리하면 더 견고하다.

- **ExplodeArea VFX/SFX 훅 부재**: ExplodeArea 실행 시 폭발 이펙트를 재생할 훅이 없다. FSkillFXData의 ImpactVFX/ImpactSFX를 활용할 수 있도록 ExplodeArea에서 가상 함수 또는 델리게이트 호출 지점을 추가하면 좋다.

- **HandleExtraParametersByType의 if 체인**: 현재 MoveType 분기가 `if` / `if`로 되어 있어 독립적으로 평가된다. MoveType은 상호 배타적이므로 `else if`로 연결하는 것이 의도를 명확히 전달하고 불필요한 비교를 제거한다. (HitType AREA 분기는 MoveType과 독립이므로 별도 if 유지가 맞다.)

---

## Gemini 리뷰 요약

Gemini 리뷰에서 Claude 리뷰와 중복되지 않는 지적 사항:

- **LoadSynchronous 동기 로딩 비용**: GA 활성화 시점에 LoadSynchronous()를 호출하는 것은 싱글플레이어에서 허용 가능하나, 에셋 규모가 커지면 히칭 원인이 될 수 있다. 스킬 등록/장착 시점에 미리 비동기 로드하는 방안 검토 가능.
- **IHitStrategy 인터페이스 분리**: HitType별 처리를 Strategy 패턴으로 분리하면 BaseProjectile이 모든 HitType 로직을 알 필요가 없어져 OCP 강화 가능. 다만 현재 타입 수(SINGLE/PIERCE/AREA)에서는 오버엔지니어링이므로 타입 5개 초과 시 검토.
- **FindNearestEnemy 일반화**: Team_Enemy 태그가 하드코딩되어 있어, 향후 아군 타겟팅이 필요한 유틸 스킬에서 재사용 불가. FGameplayTagContainer를 파라미터로 받는 일반화 검토.

---

## 종합 평가

| 항목 | 점수 | 비고 |
|------|------|------|
| 패턴 적합도 | 4/5 | Template Method/Strategy 적절히 활용. HandleExtraParametersByType의 bSuccess 덮어쓰기 로직 결함 |
| 코드 가독성 | 4/5 | 헬퍼 분리 우수. 한글 주석 일관성 양호. MoveType if 체인을 else if로 개선 여지 |
| 메모리 안전성 | 3/5 | HomingTarget UPROPERTY 누락, OnPoolDeactivate HOMING 리셋 누락이 실제 버그 유발 가능 |
| 기획서 정합 | 4/5 | Area 감쇠 경계값 연산자 불일치(> vs >=). 나머지 모든 규칙은 정확히 구현됨 |
| 컨벤션 준수 | 4/5 | 중괄호 규칙 준수. SPAWN_OFFSET 하드코딩은 이전 리뷰에서도 지적된 반복 패턴 |

---

## 필수 수정 사항 요약 (머지 전 해결 필요)

1. **RuntimeDataStructs.h:183** -- HomingTarget에 UPROPERTY() 추가
2. **BaseProjectile.cpp OnPoolDeactivate** -- bIsHomingProjectile/HomingTargetComponent/ProjectileGravityScale 리셋 추가
3. **GA_ProjectileAttack.cpp HandleExtraParametersByType** -- ARC 실패 시 AREA 분기 진입 방지 (early return 또는 AND 연산)
4. **BaseProjectile.cpp ExplodeArea** -- 감쇠 경계값 `>` 를 `>=` 로 변경 (기획서 정합)
