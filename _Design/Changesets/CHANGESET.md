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

---
<!-- 신규 항목은 이 줄 아래에 추가 -->
- date: 2026-03-28
  plan: PLAN_WeaponUpgrade_Replace_v1.0
  commit: "b6b18d4 / ed1513a / e35d380 / 0587332 / 07153a4 / aa6d657 / ddf94d8"
  files:
    modified:
      - Content/ExternalSource/DT_Weapon.csv
      - Source/RoastStaffGAS/Public/Subsystems/GameDataSubsystem.h
      - Source/RoastStaffGAS/Private/Subsystems/GameDataSubsystem.cpp
      - Source/RoastStaffGAS/Public/Subsystems/EquipmentSubsystem.h
      - Source/RoastStaffGAS/Private/Subsystems/EquipmentSubsystem.cpp
      - Source/RoastStaffGAS/Public/Character/Player/RSPlayerController.h
      - Source/RoastStaffGAS/Private/Character/Player/RSPlayerController.cpp
      - Source/RoastStaffGAS/Private/UI/LevelUpWeaponSelectWidget.cpp
      - Source/RoastStaffGAS/Private/UI/WeaponSlotWidget.cpp
    created:
      - Content/Data/Weapon/DT_WeaponDamageCurve.uasset
      - Source/RoastStaffGAS/Public/UI/WeaponReplaceWidget.h
      - Source/RoastStaffGAS/Private/UI/WeaponReplaceWidget.cpp
      - Content/UI/Levelup/WBP_WeaponReplaceUI.uasset
      - _Design/Plans/active/PLAN_WeaponUpgrade_Replace_v1.0.md
    deleted: []
  summary: "무기 강화 로직 + 교체 UI — CurveTable 기반 Lv2/Lv3 데미지 스케일링, EquipmentSubsystem 강화 판정 내재화"
  status: COMMITTED
  bugs_found: []
  bugs_fixed: []

- date: 2026-03-28
  plan: PLAN_EXP_LevelUp_UI_v1.0
  commit: "82444aa / e7a5cb6 / 501862b"
  files:
    modified:
      - Source/RoastStaffGAS/Public/Subsystems/LevelUpSubsystem.h
      - Source/RoastStaffGAS/Private/Subsystems/LevelUpSubsystem.cpp
      - Source/RoastStaffGAS/Private/Subsystems/StageManagerSubsystem.cpp
      - Source/RoastStaffGAS/Public/UI/Player/PlayerStatusBarWidget.h
      - Source/RoastStaffGAS/Private/UI/Player/PlayerStatusBarWidget.cpp
      - Content/UI/Player/WBP_PlayerStatusBar.uasset
      - Content/UI/WBP_HUD.uasset
    created:
      - _Design/Plans/active/PLAN_EXP_LevelUp_UI_v1.0.md
    deleted: []
  summary: "에너미 사망 → 스테이지 시스템 경유 EXP 전달 + PlayerStatusBarWidget EXP 바 Lerp 표시"
  status: COMMITTED
  bugs_found: []
  bugs_fixed: []

- date: 2026-03-28
  plan: PLAN_PlayerHPBarWidget_v1.0
  commit: "714cff4"
  files:
    modified: []
    created:
      - Source/RoastStaffGAS/Public/UI/Player/PlayerHPBarWidget.h
      - Source/RoastStaffGAS/Private/UI/Player/PlayerHPBarWidget.cpp
    deleted: []
  summary: "PlayerHPBarWidget 구현 — ASC HP 감지, Ghost/Health 보간, LowHealth/HitShake 애니메이션"
  status: COMMITTED
  bugs_found: []
  bugs_fixed: []

- date: 2026-03-27
  plan: PLAN_ManualFireBugFix_v1.0
  commit: "3e21a68"
  files:
    modified:
      - Source/RoastStaffGAS/Private/Subsystems/UIManagerSubsystem.cpp
      - Source/RoastStaffGAS/Private/Character/Player/RSPlayerController.cpp
      - Source/RoastStaffGAS/Private/Subsystems/EquipmentSubsystem.cpp
      - Source/RoastStaffGAS/Private/UI/LevelUpWeaponSelectWidget.cpp
      - Source/RoastStaffGAS/Public/UI/LevelUpWeaponSelectWidget.h
      - Content/GAS/GA/GA_ProjectileAttackBP.uasset
      - Content/GAS/GA/GA_SummonBaseBP.uasset
    created: []
    deleted: []
  summary: "수동 발사 클릭 미인식 버그 수정 — InputMode 정책 + NativeOnInitialized 이동"
  status: COMMITTED
  bugs_found: []
  bugs_fixed:
    - "NotifyInputModeChange GameOnly 전환 시 마우스 캡처 해제로 클릭 미인식"
    - "LevelUpWeaponSelectWidget NativeConstruct 중복 바인딩"
