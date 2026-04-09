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

## [FEATURE] TransitionGameMode FinishLoading 타이밍 수정 | PLAN_TransitionFinishLoading_v1.0
> 시작: 2026-04-09 | 기획서: 인트로_트랜지션 시스템 기획 v1.0.md

  ### [MODULE-1] RSTransitionGameMode 정리 ✓ DONE 2026-04-09
  수정: RSTransitionGameMode.cpp, RSTransitionGameMode.h
    - [x] Tick() FakeProgress 블록(bIsLoadingLevel 조건) 제거                  [P0]
    - [x] OnLevelPreloadCompleted()에서 FinishLoading() 호출 제거              [P0]
    - [x] OnLevelPreloadCompleted()에서 1초 타이머 제거 → 즉시 OpenNextLevelLatent() [P0]
    - [x] bIsLoadingLevel 멤버변수 + 연관 로직 제거 (FakeProgress 전용이었으면) [P0]

  ### [MODULE-2] RSLoadingWidget CloseUI 분리 확인
  수정: RSLoadingWidget.h, RSLoadingWidget.cpp
    - [ ] FinishLoading()이 CloseUI 호출하지 않음 확인 (Progress 1.f 표시만)  [P0]
    - [ ] IsVisible() Guard — 이미 닫힌 상태 재호출 방지                       [P0]

  ### [MODULE-3] RSGameMode OnPreWarmCompleted CloseUI 추가
  수정: RSGameMode.cpp
    - [ ] OnPreWarmCompleted()에서 FinishLoading() 후 UMS::CloseUIByID(LOADING) 호출 [P0]
    - [ ] PreWarmList 비어있을 때(즉시 StartStageFlow) CloseUI 경로 방어 처리  [P0]

## [FEATURE] Boss HP Bar UI 파이프라인 | PLAN_BossHPBar_v1.0
> 시작: 2026-04-09 | 기획서: AI_에너미 시스템 기획 v1.1.md, UI관리 시스템 기획 v1.1.md

  ### [MODULE-1] EnumExtension — EUIID::BOSS_HP_BAR 추가 ✓ DONE 2026-04-09
  수정: EnumUITypes.h
    - [x] EUIID enum에 BOSS_HP_BAR 열거값 추가                                [P0]

  ### [MODULE-2] BossHPBarWidget C++ 클래스 신규 ✓ DONE 2026-04-09
  신규: BossHPBarWidget.h, BossHPBarWidget.cpp
    - [x] URSBaseWidget 상속, UILayer=PERSISTENT 생성자 설정                  [P0]
    - [x] BindToASC(UAbilitySystemComponent*, float InPhase2Ratio) 구현        [P0]
    - [x] OnHealthChanged — ProgressBar 비율 갱신                              [P0]
    - [x] UPROPERTY 바인딩: PBar_BossHP, Anim_FadeOut 선언                    [P0]
    - [x] NativeDestruct에서 ASC 델리게이트 구독 해제                         [P0]
    - [x] bIsClosing 플래그로 이중 CloseUI 방지                               [P0]

  ### [MODULE-3] EnemySpawner + BossEnemy 연동 ✓ DONE 2026-04-09
  수정: EnemySpawner.cpp, BossEnemy.h, EnemySpawner.h
    - [x] ABossEnemy에 GetPhase2HPRatio() getter 추가                         [P0]
    - [x] case BOSS: OpenUIByID + BindToASC 호출 (TODO 교체)                  [P0]
    - [x] OnBossKilled(): bIsClosing 체크 후 CloseUIByID (TODO 교체)          [P0]
    - [x] EnemySpawner.cpp에 BossHPBarWidget.h + UIManagerSubsystem.h include [P0]

  ### [MODULE-4] WBP_BossHPBar UMG 위젯 제작 [EDITOR]
    - [ ] [EDITOR] WBP_BossHPBar 생성, Parent = BossHPBarWidget (C++)         [P0]
    - [ ] [EDITOR] PBar_BossHP ProgressBar 배치 (C++ 이름 일치 필수)          [P0]
    - [ ] [EDITOR] Anim_FadeOut 애니메이션 생성                               [P0]
    - [ ] [EDITOR] 화면 상단 중앙 앵커 배치                                   [P0]

  ### [MODULE-5] UIManagerSettings 매핑 등록 [EDITOR]
    - [ ] [EDITOR] UIClassMap: BOSS_HP_BAR → WBP_BossHPBar                    [P0]
    - [ ] [EDITOR] UILayerMap: BOSS_HP_BAR → PERSISTENT                       [P0]

  ### [MODULE-6] Phase2 시각 피드백
  수정: BossHPBarWidget.h, BossHPBarWidget.cpp
    - [ ] OnHealthChanged에서 Phase2 임계값 이하 최초 진입 시 색상 전환        [P1]
    - [ ] bPhase2Triggered 플래그로 중복 색상 전환 방지                       [P1]
    - [ ] [EDITOR] WBP_BossHPBar Phase2 경고색 Material/LinearColor 설정      [P1]



## [FEATURE] Enemy Ranged + Elite + Boss 시스템 | PLAN_EnemyExpansion_v1.0
> 시작: 2026-04-06 | 기획서: AI_에너미 시스템 기획 v1.1.md, 스킬 시스템 기획 v1.4.md

  ### [MODULE-1] DataTable 구조체 확장 ✓ DONE 2026-04-06
  수정: DataTableStructs.h, EnumTypes.h
    - [x] FEnemyStaticData에 bIsBoss bool 추가                              [P0]
    - [x] FEnemyExtData USTRUCT 신규 정의 (12컬럼)                          [P0]
    - [x] EAIType에 BOSS 추가                                               [P0]
    - [x] DT_EnemyExtData 에셋 생성 + DT_Enemy bIsBoss 컬럼 추가 (에디터)   [P0]

  ### [MODULE-2] AEnemyProjectile — 에너미 전용 투사체 (풀링) ✓ DONE 2026-04-06
  신규: EnemyProjectile.h/.cpp
  수정: EnemySpawner.h/.cpp
    - [x] AEnemyProjectile: AActor + IPoolableInterface 독립 구현            [P0]
    - [x] InitEnemyProjectile(Dir, Speed, Lifetime, Damage, GEClass, SourceASC) 구현 [P0]
    - [x] OnHit → 플레이어 ASC GE 직접 적용 → ReturnToPool                  [P0]
    - [x] OnPoolActivate/Deactivate 구현                                     [P0]
    - [x] EnemySpawner::InitPools에 투사체 풀 사전 등록                      [P0]
    - [x] BP_EnemySpawner에 EnemyProjectileClass 할당 (에디터)               [P0]

  ### [MODULE-3] ARangedEnemy ✓ DONE 2026-04-06
  신규: RangedEnemy.h/.cpp
    - [x] AEnemyBaseCharacter 상속 + InitializeRangedParams(float, FEnemyExtData) [P0]
    - [x] FireProjectile() 구현 (풀 고갈 시 경고 + 스킵)                    [P0]
    - [x] BP_RangedEnemy 생성 + AttackGEClass(GE_RangedAttack)/ProjectileClass 할당 (에디터) [P0]

  ### [MODULE-4] AEliteEnemy ✓ DONE 2026-04-06
  신규: EliteEnemy.h/.cpp
    - [x] AEnemyBaseCharacter 상속 + InitializeEliteParams(float, FEnemyExtData) [P0]
    - [x] FireProjectile() 구현 (ARangedEnemy 패턴 공유)                     [P0]
    - [x] MeleeCharge() / EndCharge() / HandleDeath() 구현                  [P0]
    - [x] BP_EliteEnemy 생성 + GE/ProjectileClass 할당 (에디터)              [P0]
    - [x] DT_Enemy_StaticData + DT_EnemyExtData Elite 행 추가 (에디터)       [P0]

  ### [MODULE-5] ABossEnemy ✓ DONE 2026-04-06
  신규: BossEnemy.h/.cpp
  수정: EnemySpawner.h/.cpp, EnemyAIController.h/.cpp
    - [x] AEnemyBaseCharacter 상속 + InitializeBossParams(float, FEnemyExtData) [P0]
    - [x] CheckPhaseTransition() — HP비율 감시, 1회 트리거                   [P0]
    - [x] Phase2 전환: PauseAI → Montage + FX → ActivatePhase2 → ResumeAI   [P0]
    - [x] ExecuteShockwave() — 범위 내 플레이어 ASC에 GE 적용                [P0]
    - [x] FireSpreadProjectile() — 45도 간격 8방향 투사체 (Phase2DamageMult) [P0]
    - [x] OnBossKilledDel 델리게이트 선언 + EnemySpawner 구독                [P0]
    - [x] HandleDeath() 오버라이드 — 전환 타이머 취소 + Broadcast            [P0]
    - [x] EnemyAIController: PauseAI/ResumeAI + BBKey_bIsPhase2 추가        [P0]
    - [x] BP_BossEnemy 생성 + GE/ProjectileClass 할당 (에디터)               [P0]
    - [>] Boss HUD 등록 → PLAN_BossHPBar_v1.0으로 이관                        [P0]
    - [~] DT_Enemy_StaticData Boss 행 AIType=BOSS + BT_BossEnemy — BT 생성 후 갱신 필요 [P0]

  ### [MODULE-6] GDS GetEnemyExtData 확장 ✓ DONE 2026-04-06
  수정: GameDataSubsystem.h/.cpp, GameDataConfig.h
    - [x] GetEnemyExtData(FName EnemyID, FEnemyExtData& Out) 구현            [P0]
    - [x] DT_EnemyExtData 테이블 포인터 UPROPERTY 추가                       [P0]

  ### [MODULE-7] BT 노드 + 행동트리 에셋 ✓ DONE 2026-04-07
  신규: BTTask_RangedReposition, BTTask_FireProjectile, BTTask_MeleeCharge,
        BTTask_ExecuteShockwave, BTDecorator_ShockwaveReady,
        BTDecorator_IsPhase2, BTDecorator_RandomChance (.h/.cpp 각 7쌍)
    - [x] BTTask_RangedReposition — 거리 기반 전진/후퇴                      [P1]
    - [x] BTTask_FireProjectile — FireProjectile() 호출                      [P1]
    - [x] BTTask_MeleeCharge — MeleeCharge() 호출                           [P1]
    - [x] BTTask_ExecuteShockwave — PrepareTime 선딜 + ExecuteShockwave()    [P1]
    - [x] BTDecorator_ShockwaveReady — 쿨타임 체크                           [P1]
    - [x] BTDecorator_IsPhase2 — ABossEnemy::IsPhase2() 체크                [P1]
    - [x] BTDecorator_RandomChance — 확률 판정 (EliteEnemy 돌진용)           [P1]
    - [ ] BT_RangedEnemy 에셋 구성                                           [P1]
    - [ ] BT_EliteEnemy 에셋 구성                                            [P1]
    - [ ] BT_BossEnemy 에셋 구성 (Phase1/2 분기)                            [P1]

---

## NEXT_SESSION
- [x] SR-FULL 아키텍처 리뷰 + 학습 리포트 (837e8db)                                 [P0]
- [x] SR-FULL H1~H4 핫픽스 적용 (4bbf1d6)                                          [P0]
- [ ] 로비 전환 시 크래시 재현 확인 (WeakThis 패치 적용됨)                          [P0]
- [x] L1: Enemy 투사체 발사 로직 공통 추출 (8a578b1)                               [P1]
- [x] L2: Enemy 파라미터 + GEClass/ProjectileClass Base 이동 (8a578b1)             [P1]
- [x] PoolingSubsystem FActorPoolBucket 래퍼 적용 (be6e42e)                        [P1]
- [x] 풀링 미적용 대상 초기화: BaseProjectile + BaseSummonObject                   [P1]

---

## BACKLOG
<!-- 의존성 기반 우선순위 정렬. 플랜 없음 → 착수 전 /planning 필수 -->

### [P1] 게임 루프 완성 (최우선)

<!-- #8 → #7 순서: 복귀 로직이 결과 UI의 출구이므로 #7 설계 전에 플로우 먼저 확정 -->
- [x] 스테이지 클리어 후 로비 복귀 로직 | 4bb9bcb | PLAN_StageResult_v1.0
- [x] 스테이지 결과 UI 및 결과 데이터 RDS/SGS 업데이트 | 067b08a,4bb9bcb | PLAN_StageResult_v1.0

- [x] 더미 데이터 추가 (캐릭터 15종 / 에너미 16종 / 스테이지 STAGE_3 4개) | e92f46b

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
[~] MODULE-7: RSTransitionGameMode FinishLoading 타이밍 변경 — 기획서 충돌 해소 완료(2026-04-08), 착수 가능 | [P1] | REF: PLAN_PoolingCentralize_v1.0
[~] 진화 시스템 (Evolution/Combination) — DT_Combination + 조합 체크 로직. 강화 시스템 완성 후 착수. | [P2] | REF: PLAN_WeaponUpgrade_Replace_v1.0
[~] 연속 레벨업 시 교체 UI 중첩 처리 — 기획서 미정의, 교체 UI 완성 후 별도 설계 필요 | [P2] | REF: PLAN_WeaponUpgrade_Replace_v1.0
[x] WeaponSlotWidget 강화 후 SkillIcon visibility 버그 수정 | 07153a4 | 2026-03-30
[x] SR HIGH#2 — OnWeaponSlotFull 교체UI TimeDilation — 실제 동작 확인됨. 문제 없음으로 종결 | 2026-04-02

---

## COMPLETED_LOG
<!-- compact 형식: [x] FEATURE명 | 커밋 | 날짜 | 플랜파일 -->
[x] LastPlayedStage UX 연속성 복원 | 7d586e6,34e2df6,7279505 | 2026-04-09 | PLAN_LastPlayedStageRestore_v1.0
[x] PlayerStatusBarWidget HP/EXP TextBlock 실시간 갱신 | d22fcd1 | 2026-04-09 | ad-hoc
[x] 풀링 시스템 중앙화 + AsyncPreWarm + GC리팩토링 | af3c5cd,8011f7f | 2026-04-08 | PLAN_PoolingCentralize_v1.0
[x] 스테이지 클리어 로직 + 결과 UI (WBP 포함) | 78378ee,6c7f997,067b08a,4bb9bcb | 2026-04-05~06 | PLAN_StageResult_v1.0
[x] 게임플로우 데이터(RuntimeDS/캐릭터적용) | d622281,f10dab1,783b4e7 | 2026-04-02~03 | PLAN_GameFlow_Data_v1.0
[x] OutGame 선택 UI(캐릭터·스테이지 선택) | 1a21f8c~8d68391,2944f16,71f1ac0,1ed214f,15ac6d4 | 2026-04-01~02 | PLAN_OutGame_SelectUI_v1.0
[x] 게임플로우 레벨(Intro/Transition/OutGame) | c0be61f,2118448,6c2c881,553dc01 | 2026-04-01 | PLAN_GameFlow_Levels_v1.0
[x] 게임플로우 인프라(UIManager/GameInstance) | 2e8a443,3feef02,389e200,f05f3c9,df16494,8150d93 | 2026-03-31 | PLAN_GameFlow_Infra_v1.0
[x] 무기강화+교체UI(CurveTable) | b6b18d4,ed1513a,e35d380,0587332,07153a4,aa6d657 | 2026-03-30 | PLAN_WeaponUpgrade_Replace_v1.0
[x] EXP전달-레벨업UI | 82444aa,e7a5cb6 | 2026-03-28 | PLAN_EXP_LevelUp_UI_v1.0
[x] 플레이어 HP바 위젯 | 714cff4 | 2026-03-28 | PLAN_PlayerHPBarWidget_v1.0
[x] 수동 발사 클릭 미인식 버그 픽스 | 3e21a68 | 2026-03-27 | PLAN_ManualFireBugFix_v1.0
