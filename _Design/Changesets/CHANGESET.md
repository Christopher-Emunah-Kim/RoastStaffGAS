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

- date: 2026-04-15
  plan: PLAN_PassiveSlotUI_v1.0
  modules: [MODULE-1 PassiveSlotWidget, MODULE-2 SlotContainerWidget 확장, MODULE-3 PC stub 구현]
  files:
    created:
      - Source/RoastStaffGAS/Public/UI/InGame/PassiveSlotWidget.h
      - Source/RoastStaffGAS/Private/UI/InGame/PassiveSlotWidget.cpp
    modified:
      - Source/RoastStaffGAS/Public/UI/InGame/SlotContainerWidget.h
      - Source/RoastStaffGAS/Private/UI/InGame/SlotContainerWidget.cpp
      - Source/RoastStaffGAS/Private/Character/Player/RSPlayerController.cpp
  status: PENDING_COMMIT (에디터 작업 후 커밋)

## COMMITTED

- date: 2026-04-15
  plan: PLAN_Phase1_InGame_v1.0
  commits: ["cc4ab96c4", "f0efdb971", "fd502de74", "9d449833e", "5cea35082"]
  status: COMMITTED
  summary: "refactor(GAS-Delegate) + fix(HP-Behavior) + feat(CharacterStat) + data(Passive) + chore(에디터): ASC delegate 일원화, HP 버그 수정, 스탯팝업 Base+Bonus 표시, 패시브 magnitude 원복"
  bugs_fixed:
    - GAS Multiplicative 공식 오진 — magnitude 0.2x(80%감소)→1.2x(20%증가) 원복
    - MaxHP StatUpgrade 적용 시 CurHP=MaxHP 회복 전용 처리
    - MaxHP 패시브 적용 시 CurHP 델타만큼 연동
    - 레벨업 카드 선택 후 스탯 창 미갱신 — ASC attribute delegate로 교체

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
