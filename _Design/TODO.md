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

## [FEATURE] 스테이지 클리어 로직 + 결과 UI | PLAN_StageResult_v1.0
> 시작: 2026-04-05 | 기획서: 게임 플로우 아키텍처 기획 v1.0.md, UI관리 시스템 기획 v1.1.md

  ### [MODULE-1] FStageRecord 구조체 확장 ✓ COMMITTED 78378ee 2026-04-05
  수정: DataTableStructs.h, RSGameSave.h
    - [x] FStageRecord에 bIsCleared 필드 추가                              [P0]
    - [x] 기존 사용처 컴파일 확인                                          [P0]

  ### [MODULE-2] SaveGameSubsystem 스테이지 기록 업데이트 ✓ COMMITTED 6c7f997 2026-04-05
  수정: SaveGameSubsystem.h/.cpp, RuntimeDataStructs.h
    - [x] FStageResultData 런타임 구조체 정의                              [P0]
    - [x] UpdateStageRecord() API 구현                                     [P0]
    - [x] BestTime/BestKill/bIsCleared 갱신 로직                          [P0]

  ### [MODULE-3] RSGameMode 스테이지 종료 로직 ✓ COMMITTED 6c7f997 2026-04-05
  수정: RSGameMode.h/.cpp, RSPlayerCharacter.cpp
    - [x] CheckStageClearCondition() 구현 (Tick 기반)                     [P0]
    - [x] OnStageCleared/OnStageFailed → EndStage(bool) 리팩토링          [P0]
    - [x] BeginPlay 리팩토링 (Initialize* 헬퍼 추출)                      [P0]
    - [x] OnResultConfirmed() 구현                                         [P0]
    - [x] RSPlayerCharacter::HandleDeath() → GameMode 연동                [P0]

  ### [MODULE-4~6] 결과 UI 구현 ✓ COMMITTED 067b08a 2026-04-06
  수정: RSStageResultWidget.h/.cpp, EnumUITypes.h
    - [x] URSStageResultWidget C++ 구현                                    [P0]
    - [x] EUIID::STAGE_RESULT 추가                                         [P0]
    - [x] WBP_StageResultWidget 블루프린트 생성 안내 (에디터 작업 필요)    [P0]

---

## BACKLOG
<!-- 의존성 기반 우선순위 정렬. 플랜 없음 → 착수 전 /planning 필수 -->

### [P1] 게임 루프 완성 (최우선)

<!-- #8 → #7 순서: 복귀 로직이 결과 UI의 출구이므로 #7 설계 전에 플로우 먼저 확정 -->
- [ ] 스테이지 클리어 후 로비 복귀 로직                                   [P1]
  <!-- TRANSITION 경유 OUTGAME 복귀 + RSGameMode 클리어 판정 트리거 -->

- [ ] 스테이지 결과 UI 및 결과 데이터 RDS/SGS 업데이트                    [P1]
  <!-- 클리어 시간/점수 표시 UI + SGS ClearedStageIDs 기록 + 재화 집계 준비 -->
  <!-- 의존: 로비 복귀 플로우 확정 후 진행 -->

- [ ] 더미 데이터 추가 (캐릭터 15종 / 무기 20종 / 에너미 10종)            [P1]
  <!-- DT_CharacterStatic / DT_Weapon / DT_Enemy 에디터 입력 (메쉬 없이 스탯만도 가능) -->
  <!-- 시스템 검증 및 밸런싱 기반 — 메쉬/애니 작업과 병렬 진행 가능 -->

### [P2] 기능 확장

- [ ] 재화 시스템 (스테이지 결과 → 재화 획득 → 캐릭터 해금)               [P2]
  <!-- 의존: 결과 UI(P1) + 더미 데이터(P1) — UnlockType::CURRENCY 경로 활성화 -->

- [ ] 소환형 스킬 추가 제작                                               [P2]
  <!-- 기존 GA_SummonBase 확장. 독립 작업 가능 -->

- [ ] 설정 UI 및 SaveData 연동                                            [P2]
  <!-- EUIID::SETTING 위젯 + RDS::SetSettingsData 경로 활성화 -->
  <!-- MODULE-5(EUIID 마이그레이션) 완료 후 진행 권장 -->

- [ ] 게임 배속 관리 기능                                                 [P2]
  <!-- CustomTimeDilation or WorldSettings 기반. 독립 작업 가능 -->

### [P3] 에셋 / 폴리싱

- [ ] 캐릭터 메쉬 + 애니메이션 추가 (에셋 탐색 포함)                      [P3]
  <!-- 더미 데이터(P1)와 병렬 가능. 탐색 → 임포트 → DT 연결 순서 -->

- [ ] 에너미 메쉬 + 애니메이션 추가 (에셋 탐색 포함)                      [P3]
  <!-- 위와 동일 절차 -->

- [ ] 아웃게임 UI 디자인 개선 (애니메이션 / 디테일 텍스처)                [P3]
  <!-- 기능 완성 후 폴리싱. 독립 작업 가능하나 우선순위 낮음 -->

---

## DEFERRED
<!-- "나중에" 항목. 이유+우선순위 필수 -->
[~] Txt_PlayerName 업데이트 로직 — 캐릭터 이름 시스템 미구현, BP에서만 텍스트 지정 | [P2] | REF: PLAN_PlayerHPBarWidget_v1.0
[~] 진화 시스템 (Evolution/Combination) — DT_Combination + 조합 체크 로직. 강화 시스템 완성 후 착수. | [P2] | REF: PLAN_WeaponUpgrade_Replace_v1.0
[~] 연속 레벨업 시 교체 UI 중첩 처리 — 기획서 미정의, 교체 UI 완성 후 별도 설계 필요 | [P2] | REF: PLAN_WeaponUpgrade_Replace_v1.0
[x] WeaponSlotWidget 강화 후 SkillIcon visibility 버그 수정 | 07153a4 | 2026-03-30
[x] SR HIGH#2 — OnWeaponSlotFull 교체UI TimeDilation — 실제 동작 확인됨. 문제 없음으로 종결 | 2026-04-02

---

## COMPLETED_LOG
<!-- compact 형식: [x] FEATURE명 | 커밋 | 날짜 | 플랜파일 -->
[x] 게임플로우 데이터(RuntimeDS/캐릭터적용) | d622281,f10dab1,783b4e7 | 2026-04-02~03 | PLAN_GameFlow_Data_v1.0
[x] OutGame 선택 UI(캐릭터·스테이지 선택) | 1a21f8c~8d68391,2944f16,71f1ac0,1ed214f,15ac6d4 | 2026-04-01~02 | PLAN_OutGame_SelectUI_v1.0
[x] 게임플로우 레벨(Intro/Transition/OutGame) | c0be61f,2118448,6c2c881,553dc01 | 2026-04-01 | PLAN_GameFlow_Levels_v1.0
[x] 게임플로우 인프라(UIManager/GameInstance) | 2e8a443,3feef02,389e200,f05f3c9,df16494,8150d93 | 2026-03-31 | PLAN_GameFlow_Infra_v1.0
[x] 무기강화+교체UI(CurveTable) | b6b18d4,ed1513a,e35d380,0587332,07153a4,aa6d657 | 2026-03-30 | PLAN_WeaponUpgrade_Replace_v1.0
[x] EXP전달-레벨업UI | 82444aa,e7a5cb6 | 2026-03-28 | PLAN_EXP_LevelUp_UI_v1.0
[x] 플레이어 HP바 위젯 | 714cff4 | 2026-03-28 | PLAN_PlayerHPBarWidget_v1.0
[x] 수동 발사 클릭 미인식 버그 픽스 | 3e21a68 | 2026-03-27 | PLAN_ManualFireBugFix_v1.0
