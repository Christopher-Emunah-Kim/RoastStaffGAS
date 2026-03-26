# 학습 리포트 — 2026-03-25 Pierce HitType

## 이번 작업에서 드러난 학습 포인트

### 🔴 반드시 학습 필요 (리뷰에서 수정 권고된 항목)

| 키워드 | 개념 | 왜 중요한가 | 학습 자료 검색 키워드 |
|--------|------|-------------|---------------------|
| switch fall-through | `case` 블록 끝에 `break` 누락 시 다음 `case`로 실행이 흘러내림 | 현재는 `default`가 비어 있어 무증상이지만, 이후 `default`에 로직이 추가되는 순간 PIERCE 발사 시 의도하지 않은 분기가 실행된다. 컴파일러 경고(`-Wimplicit-fallthrough`)로만 감지되어 런타임에서는 발견이 매우 어렵다. | "C++ switch fallthrough Wimplicit-fallthrough UE5", "switch case break omission bug" |
| 기획서 정합 — HitCount 오염 | 물리 충돌 무시 배열(`MoveIgnoreActors`)을 카운터로 전용하면 배열에 이미 들어 있는 발사자(AvatarActor)가 카운트에 포함됨 | 첫 번째 적 타격 시 HitCount=2로 계산되어 데미지 감쇠 공식이 즉시 30%를 적용한다. 기획서 "첫 번째 대상 100%" 조건이 전혀 충족되지 않으며, PierceCount 한도 비교도 함께 오염되어 실제 관통 횟수가 설정값보다 1 적게 동작한다. | "single responsibility principle C++ data structure misuse", "pierce hit count tracking UE5 projectile" |
| 기본값 누락 (USTRUCT 멤버) | `FSkillAttackHitTypeParamsPierce`의 `PierceCount`, `DamageDecay`에 기본값 미설정 | `PierceCount=0`이면 투사체가 첫 충돌 즉시 소멸. 데이터 행이 누락된 DataTable에서 행을 로드할 때 무증상으로 이상 동작이 발생한다. | "USTRUCT default member values C++ UE5", "DataTable missing row fallback UE5" |

### 🟡 심화 학습 권장 (개선 제안 항목)

| 키워드 | 개념 | 현재 수준 | 목표 수준 |
|--------|------|-----------|-----------|
| 관심사 분리 — 데이터 구조 단일 책임 | `MoveIgnoreActors`(물리 무시 목록)와 `HitCount`(논리 카운터)는 서로 다른 책임을 가짐. 하나의 자료구조로 두 가지를 해결하면 향후 요구사항 변경 시 둘 다 영향을 받음 | 구현은 되나 두 책임이 섞여 있음 | 역할을 명확히 분리하여 별도 멤버 변수를 추가하고 각 자료구조의 책임을 문서 주석으로 명시하는 수준 |
| 하드코딩 수치 — 3회 반복 패턴 | `LaunchAngle` 클램프(-80, 80), 거리 감쇠 분기(0.3f, 0.7f, 0.4f)가 이번에도 named const/DataTable 이관 없이 잔존 | 기능 구현 후 수치가 코드에 남는 습관 | 수치 리터럴이 코드에 등장하는 순간 DataTable 필드 또는 `EditDefaultsOnly`로 이관하는 반사적 체크 수준 |
| 동기 로드 vs 비동기 로드 (UE5 에셋) | `LoadRequiredClass`가 동기 로드라면 GA 활성화 프레임에 hitch 발생 가능. 무기 장착 시점 또는 스테이지 진입 시점 프리로드가 필요 | 동기 로드로 기능 구현 완료 | 비동기 로드(`UAssetManager`, `TSoftClassPtr`) 패턴을 이해하고 장착 시점에 프리로드를 적용하는 수준 |
| OnHit vs OnBeginOverlap 분기 명시 | PIERCE 타입은 `OnBeginOverlap`에서만 처리해야 하나, `OnHit`에서 PIERCE에 대한 명시적 `return` 분기가 없으면 벽 충돌 시 예상치 못한 부수효과가 발생할 수 있음 | 암묵적으로 처리됨 | `case EHitType::PIERCE: return;` 형태의 명시적 조기 반환으로 의도를 문서화하는 수준 |

### 🟢 잘한 점 (이전 대비 성장)

- **UPROPERTY() 적용**: 이전 2회 리뷰에서 반복 지적된 `UPROPERTY` 누락 패턴이 이번 `PierceCount`, `DamageDecay`에는 올바르게 적용되었다. 핵심 취약 영역이 개선 신호를 보임.
- **풀링 재사용 오염 방지**: `OnPoolDeactivate`에서 `ClearMoveIgnoreActors()`, `bHasPierceFinished` 리셋을 빠짐없이 처리. 이전 리뷰에서 지적된 "OnPoolDeactivate 리셋 완전성" 패턴을 스스로 적용.
- **이중 ReturnToPool 방지**: `bHasPierceFinished` 플래그 설계로 다중 충돌 이벤트에서 중복 반환 문제를 선제 차단.
- **ASC 없는 오브젝트 통과 처리**: 벽/지형 등 ASC가 없는 액터에 대한 통과 처리를 명시적으로 구현.
- **투사체 정지 버그 교훈 반영**: 이전 리뷰에서 학습한 `OnComponentHit` 투사체 정지 문제를 기억하여 PIERCE 시 `ECR_Overlap`으로 전환하는 설계를 스스로 적용.

---

## 이전 리포트 대비 변화

| 항목 | 이전 상태 (HomingArcArea) | 현재 상태 (Pierce) | 변화 |
|------|--------------------------|-------------------|------|
| UPROPERTY GC 추적 | 🔴 2회 연속 지적 | 🟢 이번 리뷰에서 통과 | 개선 |
| 하드코딩 수치 | 🔴 2회 연속 미해결 | 🔴 3회 연속 (LaunchAngle, 감쇠 분기값) | 미개선, 핵심 취약 영역 격상 필요 |
| 기획서 정합 | 🟡 4/5 | 🔴 2/5 (HitCount 오염) | 악화 — 데이터 구조 선택이 기획 요구사항에 직접 영향 |
| switch 안전성 | 이전 항목 없음 | 🔴 fall-through Critical | 신규 취약점 발견 |
| 풀링 리셋 완전성 | 🟡 부분 지적 | 🟢 통과 | 개선 |
| 패턴 적합도 | 4/5 | 4/5 | 유지 |
| 가독성 | 4/5 | 4/5 | 유지 |
| 메모리 안전성 | 3/5 | 4/5 | 개선 |
| 컨벤션 준수 | 4/5 | 4/5 | 유지 |

---

## 다음 세션 전 학습 체크리스트

- [ ] **switch fall-through 방어**: 검색 키워드 `"C++ switch fallthrough Wimplicit-fallthrough"` — UE5 빌드 설정에서 이 경고가 에러로 승격되는지 확인하고, 모든 switch에서 `break`/`return`/`[[fallthrough]]` 중 하나를 명시하는 습관 체크
- [ ] **HitCount 책임 분리 구현**: 검색 키워드 `"Single Responsibility Principle C++ member variable game dev"` — `MoveIgnoreActors`는 물리 무시 전용, `int32 PierceHitCount`는 카운팅 전용으로 분리하고 `OnPoolDeactivate`에서 `PierceHitCount = 0` 리셋 추가
- [ ] **USTRUCT 기본값 규칙 내재화**: 검색 키워드 `"USTRUCT default values UE5 DataTable missing row"` — DataTable에서 로드하는 USTRUCT는 반드시 최소 동작 보장 기본값을 멤버 초기화로 설정하는 규칙 체크리스트에 추가
- [ ] **하드코딩 수치 일괄 검토**: 현재 코드베이스에서 수치 리터럴(`0.3f`, `0.7f`, `0.4f`, `-80`, `80`)을 검색하여 DataTable 이관 또는 `EditDefaultsOnly` 적용 — 검색 키워드 `"data driven design UE5 EditDefaultsOnly hardcoded values"`
- [ ] **비동기 로드 패턴 학습**: 검색 키워드 `"UAssetManager AsyncLoad TSoftClassPtr UE5"` — `LoadRequiredClass` 호출 시점을 GA 활성화에서 무기 장착 시점으로 이관하는 방법 조사
