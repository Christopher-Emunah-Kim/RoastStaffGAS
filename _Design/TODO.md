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

---

## NEXT_SESSION
(없음)

---

## BACKLOG
<!-- 의존성 기반 우선순위 정렬. 플랜 없음 → 착수 전 /planning 필수 -->
<!-- ★ 설계 기준: _Design/References/Systems/게임 시스템 개선안 v2.0.md (2026-04-17 확정) -->

### [P3] 인게임 HUD 교체 + 입력 리바인딩
<!-- 도화가 스킬 리팩터링 이후 착수 -->

- [ ] Q/E 바인딩 삭제 + 숫자키 1~6 추가 (Enhanced Input)                         [P3]
- [ ] SkillManagerSubsystem SKILL_SLOT_COUNT 2→6                                  [P3]
- [ ] 인게임 HUD 상단 바 (HP/EXP/BossHP)                                          [P3]
- [ ] 인게임 HUD 하단 바 (무기 슬롯 3 + 스킬 슬롯 6)                              [P3]

### [P3] 소서리스 스킬 6개 (두 번째 캐릭터)
<!-- 도화가 스킬 리팩터링 + HUD(P3) 완료 후 착수 -->

- [ ] 소서리스 스킬 스펙 확정 (Temp 파일 기반 보완)                               [P3]
- [ ] 속성 GE (화염 DoT / 냉기 이속감소 / 번개 감전) 구현                         [P3]
- [ ] 소서리스 스킬 6개 구현                                                       [P3]

### [P2] 기능 확장 (독립 작업 가능)

- [ ] 설정 UI 및 SaveData 연동 (EUIID::SETTING + RDS::SetSettingsData)            [P2]
- [ ] 게임 배속 관리 기능 (CustomTimeDilation or WorldSettings)                   [P2]
- [ ] 에너미 BP별 KnockdownMontage 할당 (BP_MeleeEnemy, BP_RangedEnemy, BP_EliteEnemy, BP_BossEnemy) [P2]
- [ ] 스킬 GA BP별 CastingMontage 할당 (BP_GA_Skill1~6)                          [P2]
      AnimMontage 세팅 방법:
        1. AnimMontage 열기 → Notifies 트랙 우클릭 → Add Notify → AnimNotify_SendGameplayEvent 선택
        2. 노티파이 클릭 → Details에서 Tag = "Event.Montage.HitCheck" 입력
        3. 노티파이를 히트 판정 원하는 프레임으로 드래그
        4. GA BP의 CastingMontage 슬롯에 해당 몽타주 할당

---

## DEFERRED
<!-- "나중에" 항목. 이유+우선순위 필수 -->
[~] PullVortex 파라미터 DT 컬럼화 검토 — EditDefaultsOnly 현행 유지. 스킬 파라미터 통합 후 재검토 | [P3]
[~] 도화가 1번 흩뿌리기 — 몽타주 연출 보강 후 재점검 | [P2]
[~] HawkGauge 시스템 + 호크샷 — 게이지 전용 Attribute + UI 추가 비용. 3번째 캐릭터(호크아이) 때 처리 | [P3]
[x] ChargeAndRelease 타입 (스나이프) — MODULE-7~8 완료 (4efe08474, 0eac6728c)
[x] 호크아이 스킬 전체 — MODULE-1~9 완료 (PLAN_Hawkeye_Skills_v1.0)
[~] 캐릭터/무기 해금 연결 — 해금 시스템 삭제로 불필요해짐. 방향 전환 시 재검토 | [HOLD]

---

## COMPLETED_LOG
<!-- compact 형식: [x] FEATURE명 | 커밋 | 날짜 | 플랜파일 -->
[x] 호크아이 스킬 6종 구현 (MODULE-1~9) | 4efe08474,0eac6728c,ca97eb737 | 2026-05-04 | PLAN_Hawkeye_Skills_v1.0
[x] 캐릭터 메시 인게임 연동 | 3624c6080 | 2026-05-03 | PLAN_CharacterMeshApply_v1.0
[x] 데미지 인디케이터 HUD 비네트 | 621229423,b05088290,402fa1e55 | 2026-04-29 | PLAN_DamageIndicator_v1.0
[x] SR + 학습 리포트 (CombatInfra+SkillSystemArch+SkillActivationRefactor 합산) + SR_Fix | b69e4b867,ceca6206e | 2026-04-26 | PLAN_SR_Fix_v1.0
[x] SkillActivationType 3축 분리 리팩터링 + DT_CharacterSkill 통폐합 + Pierce BUG_FIX | 73b04c3b7,d7d0e50b5,691f6e1ef,eea16f8d6,93b86a62e,83b270b00 | 2026-04-26 | PLAN_SkillActivationRefactor_v1.0
[x] BossHPBarWidget HUD 자식 편입 리팩터링 | 3ba19c363,72f7479b2,e4ec76175,fec8946ac | 2026-04-26 | PLAN_BossHPBarRefactor_v1.0
[x] Game ms 최적화 (AIC/BT/CMC/Anim Tick + LOD + 적 수 조정) | 8b6c5c06f,70c8790b5,888c88484,f55a7967b,0cad93d20 | 2026-04-24 | PLAN_GameMsOpt_v1.0
[x] 퍼포먼스 최적화 (SkillFX·BT 프리로드 + GC 스파이크 수정) | 15a2e7198,0734f2ab6 | 2026-04-24 | PLAN_AsyncLoadOpt_v1.0
[x] 스킬 시스템 아키텍처 개선 (ISkillEffectInterface + DT data-driven + PullVortexActor + ElementColor) | 11407aa16,9654d150e,52a2610a4,3e9b82688,ee0b6c1b2,df93b7515 | 2026-04-21~22 | PLAN_SkillSystemArch_v1.0
[x] 도화가 스킬 3·5·6번 구현 (환영의문/먹물세례/콩콩이) | b17d85a38,3bfb35458,1f5cf3bc1 | 2026-04-21~22 | ad-hoc
[x] 아웃게임 3D 로비 + 스테이지 선택 개편 | c6fd4228c,9c6e1a738 | 2026-04-21 | PLAN_OutgameLobby3D_v1.0
[x] P0 전투 인프라 (피격반응/ProjectileSpawn/GroundEffect/CC/몽타주) | bba4030c9,b60ae6522,303a59e87,385652b6e,f6b45a9bf | 2026-04-17~21 | PLAN_CombatInfra_v1.0
[x] EnemySpawner NavMesh 스폰 위치 버그 수정 + RSGameMode 스트리밍 레벨 대기 | 75ba1d80b,6ecf988c7 | 2026-04-16 | PLAN_EnemySpawnFix_v1.0
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
