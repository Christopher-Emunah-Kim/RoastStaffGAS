# TODO — RoastStaffGAS
> 에이전트+사용자 공용. 세션 시작 시 CLAUDE.md 다음으로 읽는다.
> 최신 작업이 위에.
> ★ 설계 기준: _Design/References/Systems/게임 시스템 개선안 v2.0.md (2026-04-17 확정)

## STATUS_KEY
```
[ ] OPEN | [>] ACTIVE | [x] DONE(커밋해시) | [~] DEFERRED | [!] BLOCKED
[P0] 이번세션 | [P1] 다음세션 | [P2] 백로그
```

---

## ACTIVE_WORK
<!-- 진행 중. 완료 FEATURE는 COMPLETED_LOG로 압축 이동 -->

## [FEATURE] P0 전투 인프라 | PLAN_CombatInfra_v1.0
> 시작: 2026-04-17 | 기획서: 게임 시스템 개선안 v2.0.md

### [MODULE-1] EnemySpawnFix 커밋
수정: EnemySpawner.cpp / BP_BossEnemy.uasset / WBP_BossHPBar.uasset / DefaultGame.ini
  - [x] CHANGESET.md 기록 후 커밋 (75ba1d80b, 6ecf988c7)                  [P0]

### [MODULE-2] 피격 반응 공통 구조
수정: EnemyAttributeSet.h/.cpp / EnemyBaseCharacter.h/.cpp
  - [x] UEnemyAttributeSet::PostGameplayEffectExecute 오버라이드            [P0]
  - [x] AEnemyBaseCharacter::ApplyHitReact(FVector ImpactDir) 구현         [P0]
        넉백(LaunchCharacter) + 히트스탑(CustomTimeDilation) + 이미시브 플래시
  - [ ] 에너미 머티리얼 EmissiveIntensity 파라미터 노출 (에디터)            [P0]

### [MODULE-3] ProjectileSpawn 타입 추가
수정: EnumTypes.h / DataTableStructs.h / RuntimeDataStructs.h / GameDataSubsystem.cpp / GA_CharacterSkill.h/.cpp
  - [x] ESkillActivationType에 ProjectileSpawn / GroundEffect 추가          [P0]
  - [x] FCharacterSkillStaticData에 SkillEffectID FK + FireInterval 추가    [P0]
  - [x] GDS::GetCharacterSkillExecData SkillEffectID 복합 조회 구현         [P0]
  - [x] GA_CharacterSkill::ExecuteProjectileSpawn() 구현                   [P0]

### [MODULE-4] ElementTag + SpawnSkillFX 색상 분기
수정: RSGameplayTags.h/.cpp / DefaultGame.ini / GA_CharacterSkill.cpp
  - [ ] Element.Fire / Ice / Thunder 태그 등록 (ini + Tags.h/cpp)           [P0]
  - [ ] SpawnSkillFX 시그니처 확장 + ElementColor 분기 구현                 [P0]

### [MODULE-5] ARS_GroundEffectActor 공통 클래스
신규: ARS_GroundEffectActor.h/.cpp
수정: GA_CharacterSkill.h/.cpp
  - [ ] ARS_GroundEffectActor 구현 (IPoolableInterface + Overlap GE)        [P0]
  - [ ] GA_CharacterSkill::ExecuteGroundEffect() 구현                       [P0]

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
<!-- ★ 설계 기준: _Design/References/Systems/게임 시스템 개선안 v2.0.md (2026-04-17 확정) -->

### [P0] 전투 인프라 (선행 필수)

- [ ] EnemySpawnFix 커밋 정리 (uncommitted 변경사항)                             [P0]
- [ ] 피격 반응 공통 구조 (EnemyBaseCharacter::PostGameplayEffectExecute)         [P0]
      넉백(LaunchCharacter) + 히트스탑(TimeDilation) + 이미시브 플래시
- [ ] ProjectileSpawn 타입 추가 (ESkillActivationType + GA_CharacterSkill)        [P0]
- [ ] ElementTag 컬럼 추가 (DT_CharacterSkill) + SpawnSkillFX 색상 분기           [P0]
- [ ] ARS_GroundEffectActor 공통 클래스 신규 구현                                 [P0]

### [P1] 아웃게임 연출 (3D 로비 + 스테이지 선택)

- [ ] 로비 레벨 3D 구성 (캐릭터 3명 배치 + 조명 + 카메라 앵글)                   [P1]
- [ ] 호버 아웃라인 연출 (CustomDepth + 포스트프로세스 아웃라인)                  [P1]
- [ ] 캐릭터 선택 인터랙션 (기립 애니메이션 + 카메라 블렌드)                      [P1]
- [ ] 우측 캐릭터 정보 패널 UI (닉네임 + 초상화 + 스킬 아이콘)                   [P1]
- [ ] 스테이지 선택 분할 화면 (50/50 + 호버 확장 + 클릭 연출)                    [P1]
- [ ] MAP_Settings STAGE_002 임포트 + DT_Stage 데이터 정비                        [P1]

### [P2] 도화가 스킬 6개 (첫 번째 캐릭터)
<!-- 전투 인프라(P0) 완료 후 착수 -->

- [ ] 1번 — 흩뿌리기 (InstantAoE, FX 연동)                                       [P2]
- [ ] 2번 — 해그리기 (SelfBuff + 오라 FX Attach + GE 만료 시 제거)               [P2]
- [ ] 3번 — 환영의 문 (SpawnPreview + SetActorLocation + 출발/도착 FX)            [P2]
- [ ] 4번 — 범가르기 (ProjectileSpawn Linear + 착탄 FX)                           [P2]
- [ ] 5번 — 먹물세례 (GroundEffect + 이속감소 GE)                                 [P2]
- [ ] 6번 — 콩콩이 (ProjectileSpawn Homing + 연쇄 리다이렉트)                     [P2]

### [P3] 인게임 HUD 교체 + 입력 리바인딩
<!-- 도화가 스킬(P2) 이후 착수 -->

- [ ] Q/E 바인딩 삭제 + 숫자키 1~6 추가 (Enhanced Input)                         [P3]
- [ ] SkillManagerSubsystem SKILL_SLOT_COUNT 2→6                                  [P3]
- [ ] 인게임 HUD 상단 바 (HP/EXP/BossHP)                                          [P3]
- [ ] 인게임 HUD 하단 바 (무기 슬롯 3 + 스킬 슬롯 6)                              [P3]

### [P3] 소서리스 스킬 6개 (두 번째 캐릭터)
<!-- 도화가 스킬(P2) + HUD(P3) 완료 후 착수 -->

- [ ] 소서리스 스킬 스펙 확정 (Temp 파일 기반 보완)                               [P3]
- [ ] 속성 GE (화염 DoT / 냉기 이속감소 / 번개 감전) 구현                         [P3]
- [ ] 소서리스 스킬 6개 구현                                                       [P3]

### [P2] 기능 확장 (독립 작업 가능)

- [ ] 설정 UI 및 SaveData 연동 (EUIID::SETTING + RDS::SetSettingsData)            [P2]
- [ ] 게임 배속 관리 기능 (CustomTimeDilation or WorldSettings)                   [P2]

---

## DEFERRED
<!-- "나중에" 항목. 이유+우선순위 필수 -->
[~] HawkGauge 시스템 + 호크샷 — 게이지 전용 Attribute + UI 추가 비용. 3번째 캐릭터(호크아이) 때 처리 | [P3]
[~] ChargeAndRelease 타입 (스나이프) — 신규 타입 + 전용 HUD 오버레이. 난이도 높음 | [P3]
[~] 콩콩이 연쇄 로직 — 구현 완료 후 복잡도 판단, 필요 시 단순 HomingProjectile로 대체 | [P2 후반]
[~] 호크아이 스킬 전체 — 3번째 캐릭터. P2 도화가, P3 소서리스 완료 후 | [P4]
[~] 캐릭터/무기 해금 연결 — 해금 시스템 삭제로 불필요해짐. 방향 전환 시 재검토 | [HOLD]

---

## COMPLETED_LOG
<!-- compact 형식: [x] FEATURE명 | 커밋 | 날짜 | 플랜파일 -->
[x] EnemySpawner NavMesh 스폰 위치 버그 수정 + RSGameMode 스트리밍 레벨 대기 | 미커밋(모든 MODULE 완료) | 2026-04-16 | PLAN_EnemySpawnFix_v1.0
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
