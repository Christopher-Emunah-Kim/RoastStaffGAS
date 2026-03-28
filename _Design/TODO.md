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

## [FEATURE] 플레이어 HP바 위젯 | PLAN_PlayerHPBarWidget_v1.0
> 시작: 2026-03-28 | 기획서: UI관리 시스템 기획 v1.0.md, 캐릭터 시스템 기획 v1.2.md

  ### [MODULE-1] PlayerHPBarWidget 핵심 로직 ✓ DONE 2026-03-28
  신규: PlayerHPBarWidget.h, PlayerHPBarWidget.cpp
    - [x] BindToASC(UAbilitySystemComponent*) 구현 — 초기값 조회 + 델리게이트 구독   [P0]
    - [x] OnCurrentHPChanged() 구현 — TargetHealth 갱신, PBar_Health 즉시, GhostDelayTimer 리셋   [P0]
    - [x] OnMaxHPChanged() 구현 — CurrentMaxHealth 갱신   [P0]
    - [x] UpdateGhostBar(DeltaTime) 구현 — FInterpTo → PBar_Ghost 보간   [P0]
    - [x] NativeTick() 오버라이드 — UpdateGhostBar 호출   [P0]
    - [x] NativeDestruct() 오버라이드 — ASC 델리게이트 RemoveAll   [P0]
    - [x] CheckLowHealthState() 구현 — Anim_LowHealth 재생/정지   [P0]
    - [x] CalcPercent() 헬퍼 — MaxHP=0 방어   [P0]
    - [x] 헤더 선언 정리   [P0]

  ### [MODULE-2] HitShake 트리거 ✓ DONE 2026-03-28
  수정: PlayerHPBarWidget.h, PlayerHPBarWidget.cpp
    - [x] TriggerHitShake() 구현 — HP 감소 시 Anim_HitShake PlayAnimation (재생 중이면 재시작)   [P0]
    - [x] OnCurrentHPChanged() 내 HP 감소 조건 시 TriggerHitShake() 호출 삽입   [P0]

  ### [MODULE-3] RSHUDWidget 연동 — 불필요 판단으로 제거 (PlayerHPBarWidget 자체 바인딩)
  ### [MODULE-4] RSPlayerController 바인딩 호출 — 불필요 판단으로 제거 (PlayerHPBarWidget 자체 바인딩)

  ### [~MODULE-5] Txt_PlayerName 업데이트 로직 — 이유: 캐릭터 이름 시스템 미구현, BP에서만 텍스트 지정 | [P2] | REF: 플레이어 HP바 위젯


---

## DEFERRED
<!-- "나중에" 항목. 이유+우선순위 필수 -->

---

## COMPLETED_LOG
<!-- compact 형식: [x] FEATURE명 | 커밋 | 날짜 | 플랜파일 -->
[x] 수동 발사 클릭 미인식 버그 픽스 | 3e21a68 | 2026-03-27 | PLAN_ManualFireBugFix_v1.0
