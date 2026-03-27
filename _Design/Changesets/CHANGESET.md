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
