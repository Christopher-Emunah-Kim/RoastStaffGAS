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
  plan: PLAN_SkillSlotUI_v1.0
  commit: "a8e0d582a(feat), ef8d96bcd(data), 6905263d8(fix+docs)"
  summary: "캐릭터 스킬 슬롯 UI 통합 — SlotContainerWidget 일반화 + CharacterSkillSlotWidget 신규 + 빈 슬롯 숨김 UX"
  status: COMMITTED

- date: 2026-04-14
  commit: 1795bb48d
  summary: "LoadingWidget dangling pointer + AutoFire 타이밍 버그 수정 + 디버깅 로그 정리"
  status: COMMITTED
  files.modified:
    - Source/RoastStaffGAS/Public/Core/RSGameMode.h
    - Source/RoastStaffGAS/Private/Core/RSGameMode.cpp
    - Source/RoastStaffGAS/Public/Subsystems/EquipmentSubsystem.h
    - Source/RoastStaffGAS/Private/Subsystems/EquipmentSubsystem.cpp
    - Source/RoastStaffGAS/Private/UI/Transition/RSLoadingWidget.cpp
    - Source/RoastStaffGAS/Private/Core/Transition/RSTransitionGameMode.cpp
    - Source/RoastStaffGAS/Private/Subsystems/PoolingSubsystem.cpp
    - _Design/Portfolio/DEVLOG.md
    - _Design/TODO.md

- date: 2026-04-13
  commit: fc67bd931
  summary: "SpawnPreview 다형성 — 스킬별 PreviewActorClass DT 분리 + GA FX 스폰 구현"
  status: COMMITTED
  files.modified:
    - Source/RoastStaffGAS/Public/Data/DataTableStructs.h
    - Source/RoastStaffGAS/Public/Data/RuntimeDataStructs.h
    - Source/RoastStaffGAS/Private/Subsystems/GameDataSubsystem.cpp
    - Source/RoastStaffGAS/Private/Subsystems/SkillManagerSubsystem.cpp
    - Source/RoastStaffGAS/Public/Core/RSGameMode.h
    - Source/RoastStaffGAS/Public/GAS/Abilities/GA_CharacterSkill.h
    - Source/RoastStaffGAS/Private/GAS/Abilities/GA_CharacterSkill.cpp

- date: 2026-04-13
  plan: PLAN_Phase1_InGame_v1.0
  commit: "cd024b49(M-1), 5b69c436(M-2), e0c96d2e(data), 3cff0ba(M-3+M-4), c70213e(BUG-Slots), f4ed873(M-3M-4에셋), 0105ba7(M-5+M-7), ea56362(BUG-해금+프리뷰), 734aaa4(data), d86b8b3(docs), feeffa7(chore)"
  summary: "PHASE-1 M-1~M-7 완료 — DataTable/GDS/ExecCalc/캐릭터스킬/패시브슬롯 + BUG 3건"
  status: COMMITTED

- date: 2026-04-09
  plan: PLAN_TransitionFinishLoading_v1.0
  commit: "c5588b2"
  summary: "LoadingWidget 닫힘 타이밍 수정 — TransitionGameMode FinishLoading 제거, PreWarm 완료 시 CloseUI"
  status: COMMITTED

- date: 2026-04-09
  plan: PLAN_BossHPBar_v1.0
  commit: "d43254d, b7e3b05, 02d92c1, c45823d"
  summary: "보스 HP Bar UI 파이프라인 — BossHPBarWidget 신규 + EUIID::BOSS_HP_BAR + EnemySpawner 연동"
  status: COMMITTED

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
