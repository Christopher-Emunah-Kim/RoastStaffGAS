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

- date: 2026-04-14
  plan: PLAN_LevelUpCardPool_v1.0
  commit: null
  files:
    modified:
      - Source/RoastStaffGAS/Public/Subsystems/LevelUpSubsystem.h
      - Source/RoastStaffGAS/Private/Subsystems/LevelUpSubsystem.cpp
      - Source/RoastStaffGAS/Public/UI/LevelUpWeaponSelectWidget.h
      - Source/RoastStaffGAS/Private/UI/LevelUpWeaponSelectWidget.cpp
      - Source/RoastStaffGAS/Public/Character/Player/RSPlayerController.h
      - Source/RoastStaffGAS/Private/Character/Player/RSPlayerController.cpp
      - Source/RoastStaffGAS/Public/Data/DataTableStructs.h
      - Source/RoastStaffGAS/Public/GAS/Tags/RSGameplayTags.h
      - Source/RoastStaffGAS/Private/GAS/Tags/RSGameplayTags.cpp
      - Source/RoastStaffGAS/Private/Subsystems/PassiveSlotSubsystem.cpp
      - _Design/Portfolio/DEVLOG.md
      - _Design/TODO.md
    created:
      - Content/GAS/GE/Skill/Buff/GE_Passive_Buff_ATK.uasset
      - Content/GAS/GE/Skill/Buff/GE_Passive_Buff_CritRate.uasset
      - Content/GAS/GE/Skill/Buff/GE_Passive_Buff_DEF.uasset
      - Content/GAS/GE/Skill/Buff/GE_Passive_Buff_HP.uasset
      - Content/GAS/GE/Skill/Buff/GE_Passive_Buff_MoveSpeed.uasset
    deleted: []
  summary: "feat(LevelUp): 카드풀 확장 — StatUpgrade/PassiveAdd/WeaponUpgrade/WeaponNew 혼합 + 4장 선택 + SetByCaller Magnitude"
  status: COMMITTED
  commits: ["54c0698f", "4865e460", "8dc709e02"]
  bugs_found: []
  bugs_fixed: []

## COMMITTED

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
