# CHANGESET — RoastStaffGAS
> 에이전트용 코드 변화 추적.
> 세션 시작 시: PENDING_COMMIT 항목만 확인 (COMMITTED는 건너뜀).
> 최신 항목이 위에.

## READ_STRATEGY
```
세션 시작: status=PENDING_COMMIT 항목만 읽기
compact 트리거: COMMITTED 항목 5개 초과 시
compact 방법: COMMITTED 항목 → 별도 확인 없이 제거 (Plans/completed/에 이미 반영)
```

## PENDING_COMMIT

## COMMITTED

- date: 2026-04-14
  plan: PLAN_CharacterStatPopup_v1.0
  commits: ["a6b407a34", "6a35704d7", "58de4f52b", "d2be49f02", "5d1ba8d47", "8b1e18e42"]
  status: COMMITTED
  summary: "fix(FloatingDamage) + data(Phase1-Editor) + feat(CharacterStat): 스탯 팝업 HUD 구현 + Phase1 에디터 완료"
  bugs_fixed:
    - FloatingDamageWidget PreWarm/스폰 클래스 이중 관리 제거

- date: 2026-04-14
  plan: PLAN_LevelUpCardPool_v1.0
  commits: ["54c0698f", "4865e460", "8dc709e02"]
  status: COMMITTED
  summary: "feat(LevelUp): 카드풀 확장 — StatUpgrade/PassiveAdd/WeaponUpgrade/WeaponNew 혼합 + 4장 선택"

---
<!-- 이전 항목들은 compact됨 (2026-04-14) -->

## FORMAT
```yaml
- date: YYYY-MM-DD
  plan: PLAN_[시스템명]_vX.X
  commit: null | "abc1234"
  files:
    modified: []
    created:  []
    deleted:  []
  summary: "한 줄 요약"
  status: PENDING_COMMIT | COMMITTED | REVERTED
  bugs_found: []
  bugs_fixed: []
```
