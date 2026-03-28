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

## [FEATURE] EXP전달-레벨업UI | PLAN_EXP_LevelUp_UI_v1.0
> 시작: 2026-03-28 | 기획서: 레벨업 시스템 기획 v1.2.md, AI_에너미 시스템 기획 v1.1.md, UI관리 시스템 기획 v1.0.md

  ### [MODULE-1] LevelUpSubsystem EXP 수신 ✓ DONE 2026-03-28
  수정: LevelUpSubsystem.h/.cpp, StageManagerSubsystem.cpp
    - [x] ULevelUpSubsystem에 OnEnemyKilled(FName InEnemyID) 선언 추가                  [P0]
    - [x] OnEnemyKilled 구현 — GDS.GetEnemyData → DropEXP → AddEXP                     [P0]
    - [x] StageManagerSubsystem::OnEnemyKilled에서 LevelUpSubsystem->OnEnemyKilled 호출  [P0]
    - [x] CheckLevelUp: Level GE 적용 → EXP carry-over GE 적용 순서 검증 (기존 코드 이미 올바름) [P0]

  ### [MODULE-2] PlayerStatusBarWidget EXP 바 ✓ DONE 2026-03-28
  수정: PlayerStatusBarWidget.h/.cpp
    - [x] PBar_Exp(BindWidget) + Lerp 상태 변수 헤더 추가                               [P1]
    - [x] BindToASC에서 EXP/Level 어트리뷰트 변화 델리게이트 구독                       [P1]
    - [x] OnLevelAttrChanged — LerpStartPercent 캐시                                    [P1]
    - [x] OnEXPAttrChanged — GDS.GetLevelCurveValue("RequiredEXP", Level+1) → 퍼센트 계산 [P1]
    - [x] NativeTick — FMath::FInterpTo로 PBar_Exp Lerp 처리                           [P1]
    - [x] BindToASC 초기값 렌더링 + NativeDestruct 언바인딩                             [P1]


---

## DEFERRED
<!-- "나중에" 항목. 이유+우선순위 필수 -->
[~] Txt_PlayerName 업데이트 로직 — 캐릭터 이름 시스템 미구현, BP에서만 텍스트 지정 | [P2] | REF: PLAN_PlayerHPBarWidget_v1.0

---

## COMPLETED_LOG
<!-- compact 형식: [x] FEATURE명 | 커밋 | 날짜 | 플랜파일 -->
[x] 플레이어 HP바 위젯 | 714cff4 | 2026-03-28 | PLAN_PlayerHPBarWidget_v1.0
[x] 수동 발사 클릭 미인식 버그 픽스 | 3e21a68 | 2026-03-27 | PLAN_ManualFireBugFix_v1.0
