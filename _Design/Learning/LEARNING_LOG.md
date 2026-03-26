# 학습 진행 추적 로그

> 이 파일은 `@learning-coach` 에이전트가 자동으로 갱신합니다.
> 최신 항목이 상단에 추가됩니다.

---

## 2026-03-25 — Pierce
- 주요 학습: switch fall-through, HitCount 오염 (MoveIgnoreActors 카운터 전용 오용), USTRUCT 기본값 누락, 하드코딩 수치 (3회 반복)
- 상태: 🔴 switch fall-through (신규 Critical), 🔴 기획서 정합 HitCount 오염 (Critical), 🔴 하드코딩 수치 (3회 반복), 🟡 비동기 로드 (신규 심화), 🟢 UPROPERTY 적용 (이전 반복 지적 개선), 🟢 풀링 리셋 완전성 (개선)
- 종합 점수: 패턴 4/5 | 가독성 4/5 | 메모리 4/5 | 정합 2/5 | 컨벤션 4/5
- 리포트: `reports/LEARN_20260325_Pierce.md`

---

## 2026-03-24 — HomingArcArea
- 주요 학습: UPROPERTY GC tracking, object pool state reset, bSuccess AND-chaining, boundary condition operator (>= vs >)
- 상태: 🔴 UPROPERTY GC 추적 (2회 반복), 🔴 하드코딩 수치 (2회 반복), 🟡 풀 리셋 완전성, 🟡 bSuccess 제어 흐름, 🟢 가독성/정합/컨벤션 개선, 🟢 자기주도 버그 발견 2건
- 종합 점수: 패턴 4/5 | 가독성 4/5 | 메모리 3/5 | 정합 4/5 | 컨벤션 4/5
- 리포트: `reports/LEARN_20260324_HomingArcArea.md`

---

## 2026-03-23 — PoolingAndSummon
- 주요 학습: UPROPERTY on TArray/TMap, infinite loop guard, Composed Method pattern, GE context instigator
- 상태: 🔴 UPROPERTY GC 추적 (ActorPool Critical), 🔴 하드코딩 SPAWN_OFFSET, 🔴 기획서 정합 (적 없을 때 발동 생략 미구현), 🟡 가독성, 🟡 컨벤션
- 종합 점수: 패턴 4/5 | 가독성 3/5 | 메모리 2/5 | 정합 3/5 | 컨벤션 3/5
- 리포트: (최초 기록 — 별도 리포트 없음, SR_2026-03-23_PoolingAndSummon.md 참조)
