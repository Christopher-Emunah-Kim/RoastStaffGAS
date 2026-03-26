# 학습 리포트 — 2026-03-24 HomingArcArea

## 이번 작업에서 드러난 학습 포인트

### 반드시 학습 필요 (리뷰에서 수정 권고된 항목)

| 키워드 | 개념 | 왜 중요한가 | 학습 자료 검색 키워드 |
|--------|------|-------------|---------------------|
| UPROPERTY on TWeakObjectPtr | USTRUCT 내 TWeakObjectPtr에도 UPROPERTY()가 없으면 GC가 해당 참조를 추적하지 않는다. 대상 오브젝트 수거 후 IsValid()가 신뢰할 수 없는 결과를 돌려준다. | 대상이 GC되었을 때 IsValid() 체크가 무의미해져 null 역참조 위험이 발생하므로 안전한 유도탄 타겟 추적이 불가능해진다. | "UPROPERTY TWeakObjectPtr USTRUCT UE5 GC tracking" |
| Object pool state reset completeness | 풀에서 재사용되는 액터는 이전 활성화에서 설정된 모든 컴포넌트 상태를 리셋해야 한다. bIsHomingProjectile, HomingTargetComponent, GravityScale 등 UProjectileMovementComponent의 런타임 상태가 그 대상이다. | 리셋 누락은 재사용 시 의도하지 않은 타입의 동작(LINEAR 투사체가 Homing 모드로 날아가는 등)을 만들어 재현이 어려운 버그를 낳는다. | "object pool reset state UE5 UProjectileMovementComponent reuse" |
| Control flow with multi-step boolean (bSuccess AND chaining) | 순차적으로 독립 분기가 각각 bSuccess를 덮어쓰면, 앞 단계의 실패가 뒤 단계 성공으로 마스킹된다. 각 분기 결과를 AND 연산하거나 실패 즉시 early return으로 전파해야 한다. | ARC 파라미터 없이 AREA만 설정된 불완전한 InitData가 생성되면 런타임에 예측 불가한 투사체 동작이 발생하며, 문제 원인을 추적하기 매우 어렵다. | "boolean control flow multi-step validation C++ early return pattern" |
| Boundary condition operator (>= vs >) | 기획서에서 "30% 이상"이라고 쓰면 코드는 `>= 0.3f`여야 한다. `> 0.3f`는 경계값 자체를 다른 구간에 귀속시킨다. | 경계값 처리 불일치는 기획서 검수를 통과해도 코드에서 조용히 잘못 동작한다. 숫자 하나 차이로 밸런스가 틀어진다. | "floating point boundary condition inclusive exclusive C++ game balance" |

### 심화 학습 권장 (개선 제안 항목)

| 키워드 | 개념 | 현재 수준 | 목표 수준 |
|--------|------|-----------|-----------|
| ResetProjectileState 헬퍼 패턴 | OnPoolDeactivate에서 개별 필드를 리셋하는 대신 InitProjectile 시작부에서 컴포넌트 전체를 기본값으로 되돌리는 단일 헬퍼를 두면, 새 타입 추가 시 리셋 누락 자체가 불가능해진다. | 리셋 항목을 수동으로 나열함 — 누락 위험 구조 | 구조적으로 리셋 누락이 불가능한 헬퍼 패턴 도입 |
| else if for mutually exclusive branches | if / if 연속은 상호 배타적 분기에서 불필요한 비교를 실행하고 "독립적으로 평가된다"는 의도를 전달한다. else if는 배타성을 명시적으로 코드에 인코딩한다. | if 연속 사용 중 | 상호 배타적 조건에서 else if 일관 적용 |
| Strategy pattern for HitType | HitType별 처리를 인터페이스로 분리하면 BaseProjectile이 SINGLE/PIERCE/AREA 로직을 직접 알 필요가 없어 OCP를 강화한다. 현재 타입 수(3개)에서는 시기상조이지만 5개 초과 시 고려해야 한다. | if 분기로 처리 | 타입 5개 초과 시 Strategy 패턴 판단 가능 |
| const reference parameter | 구조체를 값으로 전달하면 복사 비용이 발생한다. 읽기 전용 파라미터는 `const T&`로 선언한다. | 값 전달 사용 | 읽기 전용 구조체 파라미터에 const& 일관 적용 |
| FindNearestEnemy 재사용성 | Team_Enemy 태그를 하드코딩하면 아군 타겟팅 스킬에서 재사용 불가. FGameplayTagContainer를 파라미터로 받아 일반화하면 GA_Base 또는 유틸 레이어로 승격 가능해진다. | GA_ProjectileAttack private에 하드코딩 | 태그 파라미터화 후 공통 레이어로 승격 |

### 잘한 점 (이전 대비 성장)

- **메모리 안전성 부분 개선**: 이전 리뷰(SR_2026-03-23)에서 Critical로 지적된 ActorPool UPROPERTY() 누락 이후, 이번에는 HomingTarget 필드에 UPROPERTY 누락이 HIGH로 내려갔다. 동일 패턴을 완전히 숙달한 수준은 아니지만, 심각도 수준 하락은 인식이 높아졌음을 보여준다.
- **풀링 기본 패턴 안정화**: bHasExploded 리셋, LifetimeTimerHandle ClearTimer, InitProjectile bHasExploded = false 초기화 등 풀링 생명주기 핵심 패턴은 통과 — 이전 리뷰에서 지적된 풀링 리셋 패턴이 부분적으로 내재화되었다.
- **가독성 향상**: 이전 3/5에서 이번 4/5로 상승. 헬퍼 분리(BuildInitData, HandleHomingType 등)와 한글 주석 일관성이 인정받았다.
- **기획서 정합 향상**: 이전 3/5에서 이번 4/5로 상승. 경계값 연산자 불일치 1건 외 나머지 규칙 전부 정확히 구현.
- **컨벤션 향상**: 이전 3/5에서 이번 4/5로 상승. 인덴테이션 혼용 해소, 중괄호 규칙 완전 준수.
- **자기주도 버그 발견**: FindNearestEnemy null 체크 후 return 누락, HandleExtraParametersByType bSuccess 초기값 오류를 시니어 리뷰 전에 스스로 발견·수정했다. 이는 코드를 작성하면서 직접 버그를 캐치하는 메타인지 향상을 보여준다.

---

## 이전 리포트 대비 변화

| 항목 | 이전 상태 (SR_2026-03-23) | 현재 상태 (SR_2026-03-24) | 변화 |
|------|---------------------------|---------------------------|------|
| 패턴 적합도 | 4/5 | 4/5 | 유지 |
| 코드 가독성 | 3/5 | 4/5 | +1 향상 |
| 메모리 안전성 | 2/5 | 3/5 | +1 향상 |
| 기획서 정합 | 3/5 | 4/5 | +1 향상 |
| 컨벤션 준수 | 3/5 | 4/5 | +1 향상 |
| UPROPERTY GC 추적 | ActorPool UPROPERTY 누락 (Critical) | HomingTarget UPROPERTY 누락 (HIGH) | 반복 패턴, 심각도 하락 |
| 풀링 리셋 누락 | (이번에 신규 지적) | HOMING 상태 리셋 누락 (HIGH) | 신규 유형의 리셋 누락 — 패턴 확장 필요 |
| 하드코딩 | SPAWN_OFFSET 지적 | SPAWN_OFFSET 반복 지적 + 감쇠값 추가 지적 | 미해결 반복 패턴 |
| 자기 발견 버그 수 | 0건 | 2건 | 메타인지 개선 신호 |

---

## 반복 취약 패턴 경보

아래 두 항목은 **연속 2회 이상 리뷰에서 지적**되었다. 다음 구현 전 의식적으로 점검해야 한다.

1. **UPROPERTY GC 추적** — TObjectPtr / TWeakObjectPtr / TArray / TMap 모두, USTRUCT 포함, 선언 즉시 UPROPERTY() 여부를 확인한다.
2. **하드코딩 수치** — SPAWN_OFFSET이 2회 연속 지적. 감쇠 파라미터도 추가됨. 숫자 리터럴이 코드에 들어가는 순간 DataTable 항목 추가 또는 EditDefaultsOnly로 대체할 것.

---

## 다음 세션 전 학습 체크리스트

- [ ] UPROPERTY GC 추적 규칙 내재화: "USTRUCT TWeakObjectPtr UPROPERTY UE5" 검색 후 공식 문서 확인. 모든 신규 USTRUCT 필드 선언 시 즉시 UPROPERTY() 여부 결정.
- [ ] bSuccess AND 체이닝 패턴 연습: 두 단계 이상 검증을 거치는 함수를 작성할 때 "앞 단계 실패 시 뒤 단계가 앞 단계 결과를 덮어쓸 수 있는가?"를 묻는 습관 형성. 검색 키워드: "multi-step validation early return C++ patterns"
- [ ] 경계값 기획서 대조 체크리스트 추가: 감쇠/범위/조건 수치를 구현할 때 기획서의 "이상/초과/이하/미만" 표현과 코드의 `>=` / `>` / `<=` / `<`를 1:1 대조 후 커밋.
- [ ] OnPoolDeactivate 리셋 체크리스트 습관화: 새 MoveType/HitType 추가 후 OnPoolDeactivate에서 해당 타입이 설정한 모든 컴포넌트 상태가 리셋되는지 확인. "ResetProjectileState helper UE5 object pool" 참고.
- [ ] const& 파라미터 습관화: 구조체 파라미터 선언 시 읽기 전용이면 즉시 `const T&`로 작성. 검색: "C++ pass by const reference struct best practice"
- [ ] SPAWN_OFFSET DataTable 이관: 이미 2회 지적된 미해결 항목. 다음 세션에서 DataTable 항목 추가 또는 EditDefaultsOnly 적용.
