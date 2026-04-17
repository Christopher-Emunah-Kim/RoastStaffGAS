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




---

## NEXT_SESSION
- [x] SR-FULL 아키텍처 리뷰 + 학습 리포트 (837e8db)                                 [P0]
- [x] SR-FULL H1~H4 핫픽스 적용 (4bbf1d6)                                          [P0]
- [x] 로비 전환 시 크래시 재현 확인 (WeakThis 패치 적용됨)                          [P0]
- [x] L1: Enemy 투사체 발사 로직 공통 추출 (8a578b1)                               [P1]
- [x] L2: Enemy 파라미터 + GEClass/ProjectileClass Base 이동 (8a578b1)             [P1]
- [x] PoolingSubsystem FActorPoolBucket 래퍼 적용 (be6e42e)                        [P1]
- [x] 풀링 미적용 대상 초기화: BaseProjectile + BaseSummonObject                   [P1]

---

## BACKLOG
<!-- 의존성 기반 우선순위 정렬. 플랜 없음 → 착수 전 /planning 필수 -->
<!-- ★ 설계 기준: _Design/References/Systems/게임 시스템 개선안 v1.0.md (2026-04-10 확정) -->

### [P1] 게임 루프 완성 (최우선)

<!-- #8 → #7 순서: 복귀 로직이 결과 UI의 출구이므로 #7 설계 전에 플로우 먼저 확정 -->
- [x] 스테이지 클리어 후 로비 복귀 로직 | 4bb9bcb | PLAN_StageResult_v1.0
- [x] 스테이지 결과 UI 및 결과 데이터 RDS/SGS 업데이트 | 067b08a,4bb9bcb | PLAN_StageResult_v1.0

- [x] 더미 데이터 추가 (캐릭터 15종 / 에너미 16종 / 스테이지 STAGE_3 4개) | e92f46b

### [P1] 게임 시스템 개선안 v1.0 구현 (설계 확정 2026-04-10)
<!-- 상세 설계: _Design/References/Systems/게임 시스템 개선안 v1.0.md -->

#### PHASE 1 — 인게임 루프 완성
- [x] 무기 자동발사 전환 (수동 액티브 모드 제거, 최근접 타겟팅)             [P1]
- [x] 캐릭터 고유 스킬 시스템 (GA 2개, Q/E, ESkillActivationType ENUM)      [P1]
- [x] 데미지 공식 ExecCalc 적용 (ATK/DEF 실제 계산 반영)                    [P1]
- [x] 레벨업 선택지 확장 (정적+동적 카드 풀, 무기 최소 1장 보장)            [P1]
- [x] 패시브 슬롯 시스템 — 로직·스탯표시·슬롯 UI 완료  [P1]

#### PHASE 2 — 아웃게임 플로우 재조립
- [ ] 로비 캐릭터 선택 UI 통합                                               [P1]

#### PHASE 3 — 메타 연결
- [ ] 캐릭터/무기 해금 연결 (재화 시스템 삭제로 해금 트리거 재설계 필요)    [P2]
  <!-- 재설계 방향 미확정 — 착수 전 논의 필요 -->

### [P2] 기능 확장

- [ ] 소환형 스킬 추가 제작                                               [P2]
  <!-- 기존 GA_SummonBase 확장. 독립 작업 가능 -->

- [ ] 설정 UI 및 SaveData 연동                                            [P2]
  <!-- EUIID::SETTING 위젯 + RDS::SetSettingsData 경로 활성화 -->

- [ ] 게임 배속 관리 기능                                                 [P2]
  <!-- CustomTimeDilation or WorldSettings 기반. 독립 작업 가능 -->

### [P1] 에셋 / 폴리싱

- [ ] 캐릭터 메쉬 + 애니메이션 추가 (에셋 탐색 포함)                      [P1]
  <!-- 탐색 → 임포트 → DT 연결 순서 -->

- [ ] 에너미 메쉬 + 애니메이션 추가 (에셋 탐색 포함)                      [P1]
  <!-- 위와 동일 절차 -->

- [ ] 아웃게임 UI 디자인 개선 (애니메이션 / 디테일 텍스처)                [P1]

---

## DEFERRED
<!-- "나중에" 항목. 이유+우선순위 필수 -->
[~] BossHPBar PERSISTENT 레이어 — HUD 위젯보다 ZOrder 높아 HUD 버튼 입력 차단 가능성 있음. 현재 플레이에 영향 없으나 HUD 인터렉션 추가 시 ZOrder 조정 또는 HitTestInvisible 처리 필요 | [P2] | REF: PLAN_BossHPBar_v1.0
[~] Txt_PlayerName 업데이트 로직 — 캐릭터 이름 시스템 미구현, BP에서만 텍스트 지정 | [P2] | REF: PLAN_PlayerHPBarWidget_v1.0
[x] MODULE-7: RSTransitionGameMode FinishLoading 타이밍 변경 | c5588b2 | PLAN_TransitionFinishLoading_v1.0
[~] 진화 시스템 (Evolution/Combination) — DT_Combination + 조합 체크 로직. 강화 시스템 완성 후 착수. | [P2] | REF: PLAN_WeaponUpgrade_Replace_v1.0
[~] 연속 레벨업 시 교체 UI 중첩 처리 — 기획서 미정의, 교체 UI 완성 후 별도 설계 필요 | [P2] | REF: PLAN_WeaponUpgrade_Replace_v1.0
[x] WeaponSlotWidget 강화 후 SkillIcon visibility 버그 수정 | 07153a4 | 2026-03-30
[x] SR HIGH#2 — OnWeaponSlotFull 교체UI TimeDilation — 실제 동작 확인됨. 문제 없음으로 종결 | 2026-04-02

---

## COMPLETED_LOG
<!-- compact 형식: [x] FEATURE명 | 커밋 | 날짜 | 플랜파일 -->
[x] 캐릭터 스탯 팝업 HUD (MODULE 1~3) | 58de4f52b,d2be49f02,5d1ba8d47 | 2026-04-14 | PLAN_CharacterStatPopup_v1.0
[x] PHASE-1 인게임 루프 완성 (MODULE 1~7 + 에디터) | cd024b49~8b1e18e42 | 2026-04-13~14 | PLAN_Phase1_InGame_v1.0
[x] 패시브 슬롯 UI (PassiveSlotWidget + SlotContainer + PC 연결) | 0a9533b9b,7c36f7fd3,ab9d95674,c2a58ec1a | 2026-04-15 | PLAN_PassiveSlotUI_v1.0
[x] TransitionGameMode FinishLoading 타이밍 수정 (MODULE 1~3) | c5588b2 | 2026-04-09 | PLAN_TransitionFinishLoading_v1.0
[x] Enemy Ranged + Elite + Boss 시스템 (MODULE 1~7) | 에디터 에셋 포함 | 2026-04-06~09 | PLAN_EnemyExpansion_v1.0
[x] Boss HP Bar UI 파이프라인 (C++ MODULE 1~4) | d43254d,b7e3b05,02d92c1,c45823d | 2026-04-09 | PLAN_BossHPBar_v1.0
[x] LastPlayedStage UX 연속성 복원 | 7d586e6,34e2df6,7279505 | 2026-04-09 | PLAN_LastPlayedStageRestore_v1.0
[x] PlayerStatusBarWidget HP/EXP TextBlock 실시간 갱신 | d22fcd1 | 2026-04-09 | ad-hoc
[x] 풀링 시스템 중앙화 + AsyncPreWarm + GC리팩토링 | af3c5cd,8011f7f | 2026-04-08 | PLAN_PoolingCentralize_v1.0
[x] 캐릭터 스킬 슬롯 UI 통합 (UX 빈슬롯 숨김 + CharacterSkillSlot Q/E) | a8e0d582a,ef8d96bcd,6905263d8 | 2026-04-14 | PLAN_SkillSlotUI_v1.0
[x] 스테이지 클리어 로직 + 결과 UI (WBP 포함) | 78378ee,6c7f997,067b08a,4bb9bcb | 2026-04-05~06 | PLAN_StageResult_v1.0
[x] 게임플로우 데이터(RuntimeDS/캐릭터적용) | d622281,f10dab1,783b4e7 | 2026-04-02~03 | PLAN_GameFlow_Data_v1.0
[x] OutGame 선택 UI(캐릭터·스테이지 선택) | 1a21f8c~8d68391,2944f16,71f1ac0,1ed214f,15ac6d4 | 2026-04-01~02 | PLAN_OutGame_SelectUI_v1.0
[x] 게임플로우 레벨(Intro/Transition/OutGame) | c0be61f,2118448,6c2c881,553dc01 | 2026-04-01 | PLAN_GameFlow_Levels_v1.0
[x] 게임플로우 인프라(UIManager/GameInstance) | 2e8a443,3feef02,389e200,f05f3c9,df16494,8150d93 | 2026-03-31 | PLAN_GameFlow_Infra_v1.0
[x] 무기강화+교체UI(CurveTable) | b6b18d4,ed1513a,e35d380,0587332,07153a4,aa6d657 | 2026-03-30 | PLAN_WeaponUpgrade_Replace_v1.0
[x] EXP전달-레벨업UI | 82444aa,e7a5cb6 | 2026-03-28 | PLAN_EXP_LevelUp_UI_v1.0
[x] 플레이어 HP바 위젯 | 714cff4 | 2026-03-28 | PLAN_PlayerHPBarWidget_v1.0
[x] 수동 발사 클릭 미인식 버그 픽스 | 3e21a68 | 2026-03-27 | PLAN_ManualFireBugFix_v1.0
