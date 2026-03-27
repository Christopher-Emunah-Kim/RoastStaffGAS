# TODO — RoastStaffGAS
> 에이전트+사용자 공용. 세션 시작 시 CLAUDE.md 다음으로 읽는다.
> 최신 작업이 위에.

## STATUS_KEY
```
[ ] OPEN | [>] ACTIVE | [x] DONE(커밋해시) | [~] DEFERRED | [!] BLOCKED
[P0] 이번세션 | [P1] 다음세션 | [P2] 백로그
```

---

## ACTIVE_WORK
<!-- 진행 중. 완료 FEATURE는 COMPLETED_LOG로 압축 이동 -->

## [BUGFIX] 수동 발사 무음 실패 | PLAN_ManualFireBugFix_v1.0
> 시작: 2026-03-27 | 기획서: 없음 (버그 픽스)

  ### [MODULE-1] bRetriggerInstancedAbility 픽스 ✓ DONE 2026-03-27
  수정: Source/RoastStaffGAS/Private/GAS/Abilities/GA_ProjectileAttack.cpp
    - [x] 생성자에 `bRetriggerInstancedAbility = true` 추가   [P0]

  ### [MODULE-2] FireSlot 진단 + LastManualFireTime 게이트 ✓ DONE 2026-03-27
  수정: Source/RoastStaffGAS/Public/Data/RuntimeDataStructs.h
  수정: Source/RoastStaffGAS/Private/Subsystems/EquipmentSubsystem.cpp
    - [x] FWeaponSlotInstanceData에 LastManualFireTime 필드 추가   [P0]
    - [x] FireSlot: TriggerAbilityFromGameplayEvent 반환값 체크 + KHS_WARN   [P0]
    - [x] RequestManualFire: 0.1s LastManualFireTime 게이트 추가   [P0]

---

## DEFERRED
<!-- "나중에" 항목. 이유+우선순위 필수 -->

---

## COMPLETED_LOG
<!-- compact 형식: [x] FEATURE명 | 커밋 | 날짜 | 플랜파일 -->
