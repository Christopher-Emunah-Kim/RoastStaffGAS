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

### [BUG] FloatingDamageWidgetClass 중복 관리 [P2]
- RSGameMode.DamageFloatingWidgetClass (PreWarm용) + RSPlayerController.FloatingDamageWidgetClass (실제 스폰용) 두 곳에 동일 클래스 UPROPERTY 존재
- 한 쪽만 교체 시 PreWarm 대상과 실제 사용 클래스 불일치 잠재 버그
- 개선 방향: GameMode::BuildPreWarmList에서 PlayerController의 FloatingDamageWidgetClass를 읽어 사용, GameMode UPROPERTY 제거



## [FEATURE] PHASE-1 인게임 루프 완성 | PLAN_Phase1_InGame_v1.0
> 시작: 2026-04-10 | 기획서: 게임 시스템 개선안 v1.0.md
> 실행 순서: M-1 → M-2 → (M-3 ∥ M-4) → (M-5 ∥ M-7) → M-6

### [MODULE-1] DataTable 스키마 확장 ✓ DONE 2026-04-13 (cd024b49)
수정: Data/EnumTypes.h, DataTableStructs.h, RuntimeDataStructs.h
  - [x] ESkillActivationType / ELevelUpCardType ENUM 추가 (EnumTypes.h)
  - [x] FWeaponStaticData: EvolutionTag(FString) + IsUnlocked + UnlockCost 추가 (DataTableStructs.h)
  - [x] FCharacterSkillLevelData USTRUCT 신규 (DataTableStructs.h)
  - [x] FCharacterSkillStaticData : FTableRowBase 신규 (DataTableStructs.h)
  - [x] FPassiveStaticData : FTableRowBase 신규 (DataTableStructs.h)
  - [x] FLevelUpCardStaticData : FTableRowBase 신규 (DataTableStructs.h)
  - [x] FCharacterSkillExecData + FLevelUpCardDisplayData USTRUCT 신규 (RuntimeDataStructs.h)

### [MODULE-2] GDS 신규 DT 통합 ✓ DONE 2026-04-13 (5b69c436)
수정: GameDataConfig.h, GameDataSubsystem.h/.cpp
  - [x] GameDataConfig.h: DT_CharacterSkill / DT_LevelUpCard / DT_Passive 경로 추가
  - [x] GameDataSubsystem: 테이블 포인터 + 캐시 TMap 3종 추가
  - [x] GameDataSubsystem: 조회 함수 6개 구현 (GetCharacterSkillExecData 포함)

### [BUG] 시작 무기 슬롯 미등록 ✓ FIXED 2026-04-13 (c70213e)
  - 원인: DeinitializeSubsystem()이 Slots[] 미초기화 → 재진입 시 이전 WeaponID 잔존 → IsEmpty()=false → GetEmptySlotIndex()=INDEX_NONE
  - 수정: InitializeSubsystem()에서 Slots[i] = FWeaponSlotInstanceData() 완전 초기화 추가 (EquipmentSubsystem.cpp:31)

### [MODULE-3] 무기 자동발사 전환 ✓ DONE 2026-04-13 (3cff0ba)
수정: EquipmentSubsystem.h/.cpp, RSPlayerController.h/.cpp
  - [x] SLOT_COUNT = 3 확정 (SD1 기획 변경 — 캐릭터 스킬 2 + 무기 슬롯 3)
  - [x] RequestManualFire() 제거 + FindNearestEnemy() 추가
  - [x] StartAutoFire() / FireSlot() — 최근접 적 타겟팅으로 교체 (타겟 없으면 스킵)
  - [x] RSPlayerController: IA_Slot1-3 바인딩 제거 / IA_Attack → OnConfirm 재활용 / IA_SkillQ/E 추가

### [MODULE-4] ExecCalc 데미지 공식 ✓ DONE 2026-04-13 (3cff0ba, f4ed873)
신규: RS_DamageExecCalc.h/.cpp
수정: RSGameplayTags.h, BaseProjectile.cpp, BaseSummonObject.cpp, 에너미 4종
에디터: GE_Damage(ExecCalc 추가), GE_EnemyDamage(신규), 에너미 BP AttackGEClass 교체
  - [x] SetByCaller 태그 추가 (Data.WeaponBaseDamage / Data.EnemyAttackDamage)
  - [x] RS_DamageExecCalc — 플레이어→에너미: BaseDmg×(1+ATK/100)×CritMult
  - [x] RS_DamageExecCalc — 에너미→플레이어: max(1, EnemyDmg-DEF)
  - [x] 데미지 주입 변경 (BaseProjectile/Summon→WeaponBaseDamage, 에너미4종→EnemyAttackDamage)
  - [x] GE_Damage ExecCalc 교체 + GE_EnemyDamage 신규 생성 (에디터 완료)

### [MODULE-5] 캐릭터 스킬 시스템 ✓ DONE 2026-04-13 (0105ba7)
신규: RSCharacterSkillData.h, SkillManagerSubsystem.h/.cpp, GA_CharacterSkill.h/.cpp
수정: RSPlayerController.h/.cpp, RSPlayerCharacter.cpp, RSGameplayTags.h/.cpp
  - [x] Skill.Character.Slot1/Slot2 / Preview.Active 태그 추가
  - [x] SkillManagerSubsystem: InitializeSkills / ActivateSkillSlot / SpawnPreview 흐름
  - [x] GA_CharacterSkill: InstantAoE / SelfBuff / SpawnPreview 구현
  - [x] RSPlayerController: Q/E/Cancel 바인딩 + IsPreviewActive() 분기 (LMB Confirm / RMB Cancel)
  - [x] RSPlayerCharacter::InitializeAbilitySystem(): SkillManager 초기화 호출
  - [x] BP_RSPlayerController: IA_SkillQ/IA_SkillE/IA_SkillCancel 에셋 할당 + IMC_Player Q/E/RMB 매핑  [에디터]
  - [x] BP_GA_CharacterSkill (캐릭터별): SkillGEClass 할당                                                [에디터]

### [MODULE-7] 패시브 슬롯 시스템 ✓ DONE 2026-04-13 (0105ba7)
신규: PassiveSlotSubsystem.h/.cpp
수정: RSPlayerCharacter.cpp, RSPlayerController.h/.cpp, RSGameplayTags.h/.cpp
  - [x] Passive.SlotFull 태그 추가
  - [x] PassiveSlotSubsystem: TryAddPassive / IsSlotFull / MAX_SLOTS=4
  - [x] RSPlayerCharacter: PassiveSlotSubsystem 초기화 호출
  - [x] RSPlayerController: OnPassiveSlotChanged 구독 (U4 미해결 — HUD 위치 미확정)

### [SpawnPreview 다형성 + GA FX 스폰] ✓ DONE 2026-04-13
수정: DataTableStructs.h, RuntimeDataStructs.h, GameDataSubsystem.cpp, SkillManagerSubsystem.cpp, RSGameMode.h, GA_CharacterSkill.h/.cpp
  - [x] FCharacterSkillStaticData: PreviewFXClass 제거 → PreviewActorClass (TSoftClassPtr<ASummonPreviewObject>) 추가
  - [x] FCharacterSkillLevelData: FXClass TSoftClassPtr → TSoftObjectPtr 수정
  - [x] FCharacterSkillExecData: PreviewFXClass → PreviewActorClass 교체
  - [x] SkillManagerSubsystem: GameMode 의존성 제거 → ExecData.PreviewActorClass.LoadSynchronous() 사용
  - [x] RSGameMode: PreviewActorClass UPROPERTY·getter 제거
  - [x] GA_CharacterSkill: SpawnSkillFX 헬퍼 추가 — InstantAoE/SelfBuff/SpawnPreview 모두 FX 스폰
  - [ ] 에디터: DT_CharacterSkill 각 SpawnPreview 행에 PreviewActorClass 할당              [에디터]
  - [ ] 에디터: DT_CharacterSkill 각 행 FXClass(Niagara 에셋) 할당 + Radius 파라미터 설정  [에디터]

### [MODULE-6] 레벨업 카드풀 확장 ✓ COMMITTED 54c0698f 2026-04-14
수정: LevelUpSubsystem.h/.cpp, RSPlayerController.h/.cpp, LevelUpWeaponSelectWidget.h/.cpp, DataTableStructs.h, RSGameplayTags.h/.cpp, PassiveSlotSubsystem.cpp
  - [x] FOnCardPoolReady 델리게이트로 교체
  - [x] BuildStaticCardPool / BuildDynamicWeaponCards / EnsureWeaponCardGuarantee / PickFinalCards 구현
  - [x] OnCardSelected() 타입별 분기 구현 (StatUpgrade/PassiveAdd/WeaponUpgrade/WeaponNew)
  - [x] LevelUpWeaponSelectWidget: FLevelUpCardDisplayData 수신 + CardType별 UI 분기 + 4장 확장
  - [x] FLevelUpCardStaticData.Icon 추가 (StatUpgrade 아이콘)
  - [x] FPassiveStaticData.Magnitude + Data.PassiveMagnitude 태그 추가 (SetByCaller 수치 주입)
  - [x] Img_WeaponIcon → Img_CardIcon 이름 변경
  - [ ] 에디터: WBP_LevelUpWeaponSelectWidget 4번째 카드 UI 요소 추가 + Img_CardIcon 이름 변경  [에디터]
  - [ ] 에디터: BP GE 에셋 생성 (GE_Passive_ATKBoost 등) + DT_Passive GEClass/Magnitude 입력  [에디터]
  - [ ] 에디터: DT_LevelUpCard StatUpgrade 행 + Icon 입력                                      [에디터]





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
- [ ] 무기 자동발사 전환 (수동 액티브 모드 제거, 최근접 타겟팅)             [P1]
- [ ] 캐릭터 고유 스킬 시스템 (GA 2개, Q/E, ESkillActivationType ENUM)      [P1]
- [ ] 데미지 공식 ExecCalc 적용 (ATK/DEF 실제 계산 반영)                    [P1]
- [ ] 레벨업 선택지 확장 (정적+동적 카드 풀, 무기 최소 1장 보장)            [P1]
- [ ] 패시브 슬롯 시스템 (최대 4, 꽉 차면 풀 잠금, 배치 후 변경 불가)      [P1]

#### PHASE 2 — 아웃게임 플로우 재조립
- [ ] 로비 캐릭터 선택 UI 통합                                               [P2]
- [ ] 캐릭터 커스터마이즈 UI (시작무기 선택 + 스탯트리)                      [P2]
- [ ] 스테이지 진입 전 정보창 (에너미 미리보기 + 보상)                       [P2]

#### PHASE 3 — 메타 연결
- [ ] 재화(골드) 시스템 (SGS 저장 + 스테이지 결과 지급)                     [P2]
- [ ] 캐릭터/무기 해금 연결                                                  [P2]
- [ ] 공통 스탯 트리 구현 (DT_StatTree + 노드 해금 UI + 스킬 마일스톤)      [P2]

### [P2] 기능 확장 (구 백로그)

- [ ] 재화 시스템 (스테이지 결과 → 재화 획득 → 캐릭터 해금)               [P2]
  <!-- PHASE 3에 통합됨 — 위 항목 참조 -->

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
