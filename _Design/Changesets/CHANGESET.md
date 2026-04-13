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

## PENDING_COMMIT

- date: 2026-04-13
  plan: PLAN_Phase1_InGame_v1.0
  commit: "cd024b49(M-1), 5b69c436(M-2), e0c96d2e(data), 3cff0ba(M-3+M-4), c70213e(BUG-Slots), f4ed873(M-3M-4에셋), 0105ba7(M-5+M-7), ea56362(BUG-해금+프리뷰), 734aaa4(data), d86b8b3(docs), feeffa7(chore)"
  summary: "PHASE-1 M-1~M-7 완료 — DataTable/GDS/ExecCalc/캐릭터스킬/패시브슬롯 + BUG 3건"
  status: COMMITTED
  files.new:
    - Source/RoastStaffGAS/Public/GAS/Calculations/RS_DamageExecCalc.h
    - Source/RoastStaffGAS/Private/GAS/Calculations/RS_DamageExecCalc.cpp
    - Source/RoastStaffGAS/Public/Objects/Data/RSCharacterSkillData.h
    - Source/RoastStaffGAS/Public/Subsystems/SkillManagerSubsystem.h
    - Source/RoastStaffGAS/Private/Subsystems/SkillManagerSubsystem.cpp
    - Source/RoastStaffGAS/Public/GAS/Abilities/GA_CharacterSkill.h
    - Source/RoastStaffGAS/Private/GAS/Abilities/GA_CharacterSkill.cpp
    - Source/RoastStaffGAS/Public/Subsystems/PassiveSlotSubsystem.h
    - Source/RoastStaffGAS/Private/Subsystems/PassiveSlotSubsystem.cpp
  files.modified:
    - Source/RoastStaffGAS/Public/Data/EnumTypes.h
    - Source/RoastStaffGAS/Public/Data/DataTableStructs.h
    - Source/RoastStaffGAS/Public/Data/RuntimeDataStructs.h
    - Source/RoastStaffGAS/Public/Data/GameDataConfig.h
    - Source/RoastStaffGAS/Public/Subsystems/GameDataSubsystem.h
    - Source/RoastStaffGAS/Private/Subsystems/GameDataSubsystem.cpp
    - Source/RoastStaffGAS/Public/Subsystems/EquipmentSubsystem.h
    - Source/RoastStaffGAS/Private/Subsystems/EquipmentSubsystem.cpp
    - Source/RoastStaffGAS/Public/Subsystems/LevelUpSubsystem.h
    - Source/RoastStaffGAS/Private/Subsystems/LevelUpSubsystem.cpp
    - Source/RoastStaffGAS/Public/Character/Player/RSPlayerController.h
    - Source/RoastStaffGAS/Private/Character/Player/RSPlayerController.cpp
    - Source/RoastStaffGAS/Public/Character/Player/RSPlayerCharacter.h
    - Source/RoastStaffGAS/Private/Character/Player/RSPlayerCharacter.cpp
    - Source/RoastStaffGAS/Public/GAS/Tags/RSGameplayTags.h
    - Source/RoastStaffGAS/Private/GAS/Abilities/GA_ProjectileAttack.cpp
    - Source/RoastStaffGAS/Public/UI/LevelUpWeaponSelectWidget.h

- date: 2026-04-09
  plan: PLAN_TransitionFinishLoading_v1.0
  commit: "c5588b2"
  summary: "LoadingWidget 닫힘 타이밍 수정 — TransitionGameMode FinishLoading 제거, PreWarm 완료 시 CloseUI"
  status: COMMITTED
  files.new: []
  files.modified:
    - Source/RoastStaffGAS/Public/Core/Transition/RSTransitionGameMode.h
    - Source/RoastStaffGAS/Private/Core/Transition/RSTransitionGameMode.cpp
    - Source/RoastStaffGAS/Private/UI/Transition/RSLoadingWidget.cpp
    - Source/RoastStaffGAS/Public/Core/RSGameMode.h
    - Source/RoastStaffGAS/Private/Core/RSGameMode.cpp

- date: 2026-04-09
  plan: PLAN_BossHPBar_v1.0
  commit: "d43254d, b7e3b05, 02d92c1, c45823d"
  summary: "보스 HP Bar UI 파이프라인 — BossHPBarWidget 신규 + EUIID::BOSS_HP_BAR + EnemySpawner 연동"
  status: COMMITTED
  files.new:
    - Source/RoastStaffGAS/Public/UI/Enemy/BossHPBarWidget.h
    - Source/RoastStaffGAS/Private/UI/Enemy/BossHPBarWidget.cpp
  files.modified:
    - Source/RoastStaffGAS/Public/Data/EnumUITypes.h
    - Source/RoastStaffGAS/Public/Character/Enemy/BossEnemy.h
    - Source/RoastStaffGAS/Private/System/EnemySpawner.cpp

- date: 2026-04-09
  plan: BugFix-MaxHP-CurveTable
  commit: "a86af95"
  summary: "레벨업 시 MaxHP CurveTable 미갱신 수정 — ApplyLevelUp MaxHP/CurrentHP 갱신 + PreAttributeChange 클램핑 방어 로직"
  status: COMMITTED
  files.modified:
    - Source/RoastStaffGAS/Private/Subsystems/LevelUpSubsystem.cpp
    - Source/RoastStaffGAS/Private/GAS/Attributes/BaseAttributeSet.cpp
    - Content/Data/Character/CT_BaseStatCurve.uasset

- date: 2026-04-09
  plan: PLAN_LastPlayedStageRestore_v1.0
  commit: "7d586e6,34e2df6,7279505"
  summary: "LastPlayedStage UX 연속성 복원 — SGS API 추가 + StageSelectWidget 자동 복원"
  status: COMMITTED
  files.modified:
    - Source/RoastStaffGAS/Public/Core/RSGameSave.h
    - Source/RoastStaffGAS/Public/Subsystems/SaveGameSubsystem.h
    - Source/RoastStaffGAS/Private/Subsystems/SaveGameSubsystem.cpp
    - Source/RoastStaffGAS/Private/Core/OutGame/RSOutGamePlayerController.cpp
    - Source/RoastStaffGAS/Public/UI/OutGame/RSStageSelectWidget.h
    - Source/RoastStaffGAS/Private/UI/OutGame/RSStageSelectWidget.cpp

---
<!-- 신규 항목은 이 줄 위에 추가 -->
- date: 2026-04-08
  plan: GC+Refactor
  commit: "8011f7f"
  summary: "GC 안티패턴 청소 + ZOrder 매직넘버 제거"
  status: COMMITTED

- date: 2026-04-08
  plan: PLAN_PoolingCentralize_v1.0
  commit: "af3c5cd"
  summary: "풀링 초기화 중앙화 + AsyncPreWarm + 로딩 연동 (MODULE 1~6, 8 완료)"
  status: COMMITTED

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
- date: 2026-04-08
  plan: Hotfix-L1L2-PoolingBucket
  commit: "8a578b1,be6e42e"
  files:
    modified:
      - Source/RoastStaffGAS/Public/Character/Enemy/EnemyBaseCharacter.h
      - Source/RoastStaffGAS/Private/Character/Enemy/EnemyBaseCharacter.cpp
      - Source/RoastStaffGAS/Public/Character/Enemy/RangedEnemy.h
      - Source/RoastStaffGAS/Private/Character/Enemy/RangedEnemy.cpp
      - Source/RoastStaffGAS/Public/Character/Enemy/EliteEnemy.h
      - Source/RoastStaffGAS/Private/Character/Enemy/EliteEnemy.cpp
      - Source/RoastStaffGAS/Public/Character/Enemy/BossEnemy.h
      - Source/RoastStaffGAS/Private/Character/Enemy/BossEnemy.cpp
      - Source/RoastStaffGAS/Public/Character/Enemy/MeleeEnemy.h
      - Source/RoastStaffGAS/Public/Subsystems/PoolingSubsystem.h
      - Source/RoastStaffGAS/Private/Subsystems/PoolingSubsystem.cpp
  summary: "L1+L2 Enemy 투사체 공통화 + FActorPoolBucket으로 ActorPool GC 추적 확보"
  status: COMMITTED
  bugs_found: []
  bugs_fixed:
    - L1: Ranged/Elite/Boss 투사체 발사 로직 3중 중복 제거
    - L2: 공통 파라미터 5개 + AttackGEClass/ProjectileClass Base 이동
    - PoolingSubsystem ActorPool UPROPERTY 등록 (FActorPoolBucket 래퍼)

- date: 2026-04-08
  plan: SR-FULL_20260408
  commit: "837e8db"
  files:
    created:
      - _Design/Reviews/SR-FULL_20260408.md
      - _Design/Learning/reports/LEARN_20260408_SRFULL.md
    modified:
      - _Design/Reviews/CHANGE_LOG.md
      - _Design/Learning/LEARNING_LOG.md
  summary: "SR-FULL 아키텍처 리뷰 (3.5/5) + 학습 리포트 생성"
  status: COMMITTED
  bugs_found: []
  bugs_fixed: []

- date: 2026-04-08
  plan: Hotfix-SR-FULL-H1~H4
  commit: "4bbf1d6"
  files:
    modified:
      - Source/RoastStaffGAS/Public/Subsystems/PoolingSubsystem.h
      - Source/RoastStaffGAS/Public/GAS/Abilities/GA_Base.h
      - Source/RoastStaffGAS/Private/GAS/Abilities/GA_ProjectileAttack.cpp
      - Source/RoastStaffGAS/Public/Data/DataTableStructs.h
  summary: "SR-FULL H1~H4 핫픽스: ActorPool UPROPERTY, SPAWN_OFFSET 외부화, constexpr 타입, DataTable 기본값"
  status: COMMITTED
  bugs_found: []
  bugs_fixed:
    - H1: PoolingSubsystem::ActorPool GC 수집 위험
    - H2: GA_Base::SPAWN_OFFSET 하드코딩 4회차
    - H3: constexpr int32 = 80.f 타입 절삭
    - H4: DataTableStructs Arc/Area/Defense 기본값 누락
- date: 2026-04-07
  plan: PLAN_EnemyExpansion_v1.0
  commit: "6aa574e,21f442d,d91100d,092f785,4e16aa5,f805711,cba108d,d263f02,d5ada42"
  files:
    modified:
      - Source/RoastStaffGAS/Public/Character/Enemy/BossEnemy.h
      - Source/RoastStaffGAS/Private/Character/Enemy/BossEnemy.cpp
    created:
      - Source/RoastStaffGAS/Public/AI/BTTask_RangedReposition.h
      - Source/RoastStaffGAS/Private/AI/BTTask_RangedReposition.cpp
      - Source/RoastStaffGAS/Public/AI/BTTask_FireProjectile.h
      - Source/RoastStaffGAS/Private/AI/BTTask_FireProjectile.cpp
      - Source/RoastStaffGAS/Public/AI/BTTask_MeleeCharge.h
      - Source/RoastStaffGAS/Private/AI/BTTask_MeleeCharge.cpp
      - Source/RoastStaffGAS/Public/AI/BTTask_ExecuteShockwave.h
      - Source/RoastStaffGAS/Private/AI/BTTask_ExecuteShockwave.cpp
      - Source/RoastStaffGAS/Public/AI/BTDecorator_ShockwaveReady.h
      - Source/RoastStaffGAS/Private/AI/BTDecorator_ShockwaveReady.cpp
      - Source/RoastStaffGAS/Public/AI/BTDecorator_IsPhase2.h
      - Source/RoastStaffGAS/Private/AI/BTDecorator_IsPhase2.cpp
      - Source/RoastStaffGAS/Public/AI/BTDecorator_RandomChance.h
      - Source/RoastStaffGAS/Private/AI/BTDecorator_RandomChance.cpp
    deleted: []
  summary: "EnemyExpansion MODULE-7 — BT Task 4종 + Decorator 3종 + BossEnemy 쿨타임 추적 + 투사체 충돌채널 + Instigator 패턴 + 보스1회 스폰 + Pool Scale 보존"
  status: COMMITTED
  bugs_found: []
  bugs_fixed:
    - "EnemyProjectile ECC_Pawn 채널 — 적끼리 충돌 → ECC_GameTraceChannel1 커스텀 채널로 해결"
    - "투사체 자기 자신 즉시 충돌 — 80u 오프셋 → SetInstigator + IgnoreActorWhenMoving으로 교체"
    - "풀 재사용 시 AddDynamic ensure 오류 — RemoveDynamic 선행 호출로 수정"
    - "BossEnemy BP Scale 리셋 — SetActorTransform → SetActorLocationAndRotation"
    - "보스 중복 스폰 — bBossSpawned bool 가드"
- date: 2026-04-06
  plan: PLAN_EnemyExpansion_v1.0
  commit: "dc117ce,550074a,a6f40d1,9cb3a61,11ff81d,0a9ad42"
  files:
    modified:
      - Source/RoastStaffGAS/Public/Data/DataTableStructs.h
      - Source/RoastStaffGAS/Public/Data/EnumTypes.h
      - Source/RoastStaffGAS/Public/Data/GameDataConfig.h
      - Source/RoastStaffGAS/Public/Subsystems/GameDataSubsystem.h
      - Source/RoastStaffGAS/Private/Subsystems/GameDataSubsystem.cpp
      - Source/RoastStaffGAS/Public/System/EnemySpawner.h
      - Source/RoastStaffGAS/Private/System/EnemySpawner.cpp
      - Source/RoastStaffGAS/Public/Character/Enemy/EnemyAIController.h
      - Source/RoastStaffGAS/Private/Character/Enemy/EnemyAIController.cpp
    created:
      - Source/RoastStaffGAS/Public/Character/Enemy/RangedEnemy.h/.cpp
      - Source/RoastStaffGAS/Public/Character/Enemy/EliteEnemy.h/.cpp
      - Source/RoastStaffGAS/Public/Character/Enemy/BossEnemy.h/.cpp
      - Source/RoastStaffGAS/Public/Objects/Projectile/EnemyProjectile.h/.cpp
      - _Design/Plans/active/PLAN_EnemyExpansion_v1.0.md
    deleted: []
  summary: "EnemyExpansion — ARangedEnemy/AEliteEnemy/ABossEnemy + AEnemyProjectile 풀링 + EnemyAIController PauseAI/ResumeAI (MODULE-1~6 완료, MODULE-7 다음 세션)"
  status: COMMITTED
  bugs_found: []
  bugs_fixed: []
- date: 2026-04-06
  plan: null
  commit: "e92f46b"
  files:
    modified:
      - Source/RoastStaffGAS/Public/Data/EnumUITypes.h
      - Content/Data/Character/DT_Character_Static_Data.uasset
      - Content/Data/Enemy/DT_Enemy_Static_Data.uasset
      - Content/Data/Stage/DT_Stage_Static_Data.uasset
      - Content/Data/Stage/DT_Wave_Static_Data.uasset
      - ExternalSource/DT_Character_Static_Data.csv
      - ExternalSource/DT_Enemy_Static_Data.csv
      - ExternalSource/DT_Stage_Static_Data.csv
      - ExternalSource/DT_Wave_Static_Data.csv
    created: []
    deleted: []
  summary: "더미 데이터 추가 — 캐릭터 6→15종 / 에너미 2→16종 / 스테이지 STAGE_3(STG_009~012) + 웨이브 12행 / ELevelName::STAGE_3 추가"
  status: COMMITTED
  bugs_found: []
  bugs_fixed: []
- date: 2026-04-06
  plan: PLAN_StageResult_v1.0
  commit: "4bb9bcb,3c93d19,59551fb"
  files:
    modified:
      - Source/RoastStaffGAS/Public/Core/RSGameMode.h
      - Source/RoastStaffGAS/Private/Core/RSGameMode.cpp
    created: []
    deleted: []
  summary: "[StageResult 버그픽스+리팩] EndStage UI 연동 누락 수정 + AutoFire 타이머 크래시 수정 + 게임 일시정지 + EndStage 헬퍼 분리 (StopStageActivities/BuildResultData/SaveResult/ShowResultUI)"
  status: COMMITTED
  bugs_found:
    - "EndStage가 결과 UI 없이 OnResultConfirmed() 직접 호출 — 결과 화면 미표시"
    - "AutoFire 타이머 미정리 → 레벨 전환 중 ASC->GetAvatarActor() 크래시"
  bugs_fixed:
    - "StopAllFire() 선행 호출로 크래시 수정"
    - "ShowResultUI 헬퍼에서 OpenUIByID + SetResultData + 델리게이트 바인딩"
    - "SetGamePaused(true/false)로 결과 UI 중 게임 일시정지"
- date: 2026-04-06
  plan: PLAN_StageResult_v1.0
  commit: "067b08a"
  files:
    modified:
      - Source/RoastStaffGAS/Public/Data/EnumUITypes.h
      - .claude/settings.local.json
    created:
      - Source/RoastStaffGAS/Public/UI/InGame/RSStageResultWidget.h
      - Source/RoastStaffGAS/Private/UI/InGame/RSStageResultWidget.cpp
    deleted: []
  summary: "[MODULE-4~6] 스테이지 결과 UI 구현 — URSStageResultWidget C++ 위젯 + EUIID::STAGE_RESULT enum + WBP_StageResultWidget 블루프린트 생성 안내 (에디터 작업 필요)"
  status: COMMITTED
  bugs_found: []
  bugs_fixed: []
- date: 2026-04-05
  plan: PLAN_StageResult_v1.0
  commit: "8f7c6b6,78378ee,6c7f997"
  files:
    modified:
      - Source/RoastStaffGAS/Public/Data/DataTableStructs.h
      - Source/RoastStaffGAS/Public/Core/RSGameSave.h
      - Source/RoastStaffGAS/Public/Data/RuntimeDataStructs.h
      - Source/RoastStaffGAS/Public/Subsystems/SaveGameSubsystem.h
      - Source/RoastStaffGAS/Private/Subsystems/SaveGameSubsystem.cpp
      - Source/RoastStaffGAS/Public/Core/RSGameMode.h
      - Source/RoastStaffGAS/Private/Core/RSGameMode.cpp
      - Source/RoastStaffGAS/Private/Character/Player/RSPlayerCharacter.cpp
      - .claude/skills/coding/SKILL.md
      - .claude/skills/commit/SKILL.md
      - .git/hooks/pre-commit
    created:
      - _Design/Plans/active/PLAN_StageResult_v1.0.md
    deleted: []
  summary: "[MODULE-1~3] 스테이지 클리어 로직 완료 — FStageRecord/FStageResultData + SaveGameSubsystem::UpdateStageRecord() + RSGameMode::Tick클리어판정+EndStage리팩토링+BeginPlay리팩토링 + RSPlayerCharacter사망연동 | coding스킬 v3.5.0 (자동리팩토링) + commit스킬 규칙 개선"
  status: COMMITTED
  bugs_found: []
  bugs_fixed: []
- date: 2026-04-03
  plan: PLAN_GameFlow_Data_v1.0
  commit: "783b4e7"
  files:
    modified:
      - Source/RoastStaffGAS/Public/Character/Player/RSPlayerController.h
      - Source/RoastStaffGAS/Private/Character/Player/RSPlayerController.cpp
    created:  []
    deleted:  []
  summary: "[MODULE-5] RSPlayerController 인게임 UI EUIID 마이그레이션 — OpenUI<T>/TSubclassOf 제거, OpenUIByID+CloseUIByID 전환"
  status: COMMITTED
  bugs_found: []
  bugs_fixed: []
- date: 2026-04-02
  plan: PLAN_GameFlow_Data_v1.0
  commit: "5099b42 / 44567fd / a6ade9f"
  files:
    modified:
      - Source/RoastStaffGAS/Public/Data/RuntimeDataStructs.h
      - Source/RoastStaffGAS/Public/Subsystems/GameDataSubsystem.h
      - Source/RoastStaffGAS/Private/Subsystems/GameDataSubsystem.cpp
      - Source/RoastStaffGAS/Public/Subsystems/SaveGameSubsystem.h
      - Source/RoastStaffGAS/Private/Subsystems/SaveGameSubsystem.cpp
      - Source/RoastStaffGAS/Public/Core/Transition/RSTransitionGameMode.h
      - Source/RoastStaffGAS/Private/Core/Transition/RSTransitionGameMode.cpp
      - Source/RoastStaffGAS/Private/Core/OutGame/RSOutGamePlayerController.cpp
    created:
      - Source/RoastStaffGAS/Public/Subsystems/RuntimeDataSubsystem.h
      - Source/RoastStaffGAS/Private/Subsystems/RuntimeDataSubsystem.cpp
    deleted: []
  summary: "MODULE-4: RuntimeDataSubsystem SSOT 신규 + 프리로드 번들 API + TransitionGameMode 비동기 프리로드 + OGPC RDS 연동"
  status: COMMITTED
  bugs_found: []
  bugs_fixed: []
- date: 2026-04-02
  plan: BugFix_SummonMultiSlot
  commit: "a7b77c7"
  files:
    modified:
      - Source/RoastStaffGAS/Public/Subsystems/EquipmentSubsystem.h
      - Source/RoastStaffGAS/Private/Subsystems/EquipmentSubsystem.cpp
      - Source/RoastStaffGAS/Private/GAS/Abilities/GA_SummonBase.cpp
    created:  []
    deleted:  []
  summary: "소환형 무기 2슬롯 동시 장착 버그 수정 — CheckIsActiveSlot/OnSummonAbilityEnded SkillID→SpecHandle 교체"
  status: COMMITTED
  bugs_found:
    - "CheckIsActiveSlot: SkillID 기반 슬롯 탐색 → SkillID 공유 시 슬롯 0만 읽어 오판"
    - "OnSummonAbilityEnded: SkillID 기반 → 슬롯 1의 쿨타임 타이머 미재시작"
  bugs_fixed:
    - "두 함수 모두 AbilitySpecHandle 기반 매칭으로 교체"
- date: 2026-04-02
  plan: PLAN_GameFlow_Data_v1.0
  commit: "d622281 / f10dab1"
  files:
    modified:
      - Source/RoastStaffGAS/Public/Core/RSGameMode.h
      - Source/RoastStaffGAS/Private/Core/RSGameMode.cpp
      - Source/RoastStaffGAS/Public/Character/Player/RSPlayerState.h
      - Source/RoastStaffGAS/Private/Character/Player/RSPlayerState.cpp
      - Source/RoastStaffGAS/Public/Data/DataTableStructs.h
    created:  []
    deleted:  []
  summary: "RSGameMode::BeginPlay — SGS/GI 기반 CharID/StageID 적용 + InitDefaultWeapon + RSPlayerState::ApplyCharacterStats (전체 스탯 DT화, ApplyBaseStats 제거)"
  status: COMMITTED
  bugs_found: []
  bugs_fixed: []
- date: 2026-04-02
  plan: PLAN_OutGame_SelectUI_v1.0
  commit: null
  files:
    modified:
      - Source/RoastStaffGAS/Public/Subsystems/UIManagerSubsystem.h
      - Source/RoastStaffGAS/Private/Subsystems/UIManagerSubsystem.cpp
      - Source/RoastStaffGAS/Public/UI/OutGame/RSLobbyWidget.h
      - Source/RoastStaffGAS/Private/UI/OutGame/RSLobbyWidget.cpp
      - Source/RoastStaffGAS/Public/UI/OutGame/RSCharacterSelectWidget.h
      - Source/RoastStaffGAS/Private/UI/OutGame/RSCharacterSelectWidget.cpp
      - Source/RoastStaffGAS/Public/Data/EnumTypes.h
      - Source/RoastStaffGAS/Public/Core/OutGame/RSOutGamePlayerController.h
      - Source/RoastStaffGAS/Private/Core/OutGame/RSOutGamePlayerController.cpp
    created:
      - Source/RoastStaffGAS/Public/UI/OutGame/RSStageNodeWidget.h
      - Source/RoastStaffGAS/Private/UI/OutGame/RSStageNodeWidget.cpp
      - Source/RoastStaffGAS/Public/UI/OutGame/RSStageSelectWidget.h
      - Source/RoastStaffGAS/Private/UI/OutGame/RSStageSelectWidget.cpp
    deleted: []
  summary: "StageSelectWidget 노드맵 + CharSelect→StageSelect 순차 플로우 재설계 (LobbyWidget StageSelect 제거)"
  status: COMMITTED
  commit: "2944f16 / 71f1ac0 / 1ed214f / 15ac6d4"
  bugs_found: []
  bugs_fixed: []
- date: 2026-04-01
  plan: PLAN_OutGame_SelectUI_v1.0 + PLAN_GameFlow_Levels_v1.0
  commit: "c0be61f / 2118448 / 1a21f8c / 881f2db / c423f60 / 4a9b984 / 3ff34cb / 7e04f5f / 330fb4c / 47713c3 / 8d68391"
  files:
    modified:
      - Source/RoastStaffGAS/Public/Data/DataTableStructs.h
      - Source/RoastStaffGAS/Public/Data/GameDataConfig.h
      - Source/RoastStaffGAS/Public/Subsystems/GameDataSubsystem.h
      - Source/RoastStaffGAS/Private/Subsystems/GameDataSubsystem.cpp
      - Source/RoastStaffGAS/Public/Data/EnumTypes.h
      - Source/RoastStaffGAS/Public/Data/EnumUITypes.h
      - Source/RoastStaffGAS/Private/Core/RSGameInstance.cpp
    created:
      - Source/RoastStaffGAS/Public/Core/OutGame/RSOutGameMode.h/.cpp
      - Source/RoastStaffGAS/Public/Core/OutGame/RSOutGamePlayerController.h/.cpp
      - Source/RoastStaffGAS/Public/Core/RSGameSave.h/.cpp
      - Source/RoastStaffGAS/Public/Subsystems/SaveGameSubsystem.h/.cpp
      - Source/RoastStaffGAS/Public/UI/OutGame/RSLobbyWidget.h/.cpp
      - Source/RoastStaffGAS/Public/UI/OutGame/RSCharacterEntryWidget.h/.cpp
      - Source/RoastStaffGAS/Public/UI/OutGame/RSCharacterSelectWidget.h/.cpp
      - Source/RoastStaffGAS/Public/UI/OutGame/RSCharacterGridPopupWidget.h/.cpp
      - _Design/Plans/active/PLAN_OutGame_SelectUI_v1.0.md
    deleted: []
  summary: "OutGame SelectUI 전체 — OutGameMode/PC + SaveGameSubsystem + 캐릭터선택/그리드/로비 UI + DataSchema 확장"
  status: COMMITTED
  bugs_found: []
  bugs_fixed:
    - "SGS 신규 저장 시 UnlockedCharIDs 빈 배열 → DEFAULT UnlockType 체크 추가로 항상 해금 처리"

- date: 2026-04-01
  plan: PLAN_GameFlow_Levels_v1.0
  commit: "0f47105 / 6c2c881 / 553dc01 / 2d507f1 / ca83d9f"
  files:
    modified:
      - Source/RoastStaffGAS/Public/Subsystems/UIManagerSubsystem.h
      - Source/RoastStaffGAS/Private/Subsystems/UIManagerSubsystem.cpp
      - Source/RoastStaffGAS/Private/Core/RSGameInstance.cpp
      - Config/DefaultEngine.ini
      - Config/DefaultGame.ini
      - _Design/Reviews/CHANGE_LOG.md
    created:
      - Source/RoastStaffGAS/Public/Core/Intro/RSIntroGameMode.h
      - Source/RoastStaffGAS/Private/Core/Intro/RSIntroGameMode.cpp
      - Source/RoastStaffGAS/Public/Core/Intro/RSIntroPlayerController.h
      - Source/RoastStaffGAS/Private/Core/Intro/RSIntroPlayerController.cpp
      - Source/RoastStaffGAS/Public/UI/Intro/RSIntroWidget.h
      - Source/RoastStaffGAS/Private/UI/Intro/RSIntroWidget.cpp
      - Source/RoastStaffGAS/Public/UI/Intro/RSTitleWidget.h
      - Source/RoastStaffGAS/Private/UI/Intro/RSTitleWidget.cpp
      - Source/RoastStaffGAS/Public/Core/Transition/RSTransitionGameMode.h
      - Source/RoastStaffGAS/Private/Core/Transition/RSTransitionGameMode.cpp
      - Source/RoastStaffGAS/Public/Core/Transition/RSTransitionPlayerController.h
      - Source/RoastStaffGAS/Private/Core/Transition/RSTransitionPlayerController.cpp
      - Source/RoastStaffGAS/Public/UI/Transition/RSLoadingWidget.h
      - Source/RoastStaffGAS/Private/UI/Transition/RSLoadingWidget.cpp
    deleted: []
  summary: "GameFlow Levels MODULE-1~2: Intro/Transition GM+PC+Widget + OpenUIByID 반환타입 + OpenLevel URL 수정"
  status: COMMITTED
  bugs_found: []
  bugs_fixed:
    - "OpenLevel TEXT(\"TRANSITION\") 하드코딩 → MapSettings ShortName으로 수정"
    - "OpenLevel GetLongPackageName() Invalid URL → GetShortName()으로 수정"

- date: 2026-03-31
  plan: PLAN_GameFlow_Infra_v1.0
  commit: null
  files:
    modified:
      - Source/RoastStaffGAS/Public/UI/RSBaseWidget.h
      - Source/RoastStaffGAS/Public/Subsystems/UIManagerSubsystem.h
      - Source/RoastStaffGAS/Private/Subsystems/UIManagerSubsystem.cpp
      - Config/DefaultEngine.ini
    created:
      - _Design/Plans/active/PLAN_GameFlow_Infra_v1.0.md
      - _Design/Plans/active/PLAN_GameFlow_Levels_v1.0.md
      - _Design/Plans/active/PLAN_GameFlow_Data_v1.0.md
      - Source/RoastStaffGAS/Public/Data/EnumUITypes.h
      - Source/RoastStaffGAS/Public/System/UIManagerSettings.h
      - Source/RoastStaffGAS/Private/System/UIManagerSettings.cpp
      - Source/RoastStaffGAS/Public/System/MapSettings.h
      - Source/RoastStaffGAS/Private/System/MapSettings.cpp
      - Source/RoastStaffGAS/Public/Core/RSGameInstance.h
      - Source/RoastStaffGAS/Private/Core/RSGameInstance.cpp
    deleted: []
  summary: "UI 4레이어(EUIID) + UIManagerSettings + RSGameInstance 레벨전환 인프라 구축"
  status: COMMITTED
  commits: "2e8a443 / 3feef02 / 389e200 / f05f3c9 / df16494 / 8150d93"
  bugs_found: []
  bugs_fixed: []
- date: 2026-03-28
  plan: PLAN_WeaponUpgrade_Replace_v1.0
  commit: "b6b18d4 / ed1513a / e35d380 / 0587332 / 07153a4 / aa6d657 / ddf94d8"
  files:
    modified:
      - Content/ExternalSource/DT_Weapon.csv
      - Source/RoastStaffGAS/Public/Subsystems/GameDataSubsystem.h
      - Source/RoastStaffGAS/Private/Subsystems/GameDataSubsystem.cpp
      - Source/RoastStaffGAS/Public/Subsystems/EquipmentSubsystem.h
      - Source/RoastStaffGAS/Private/Subsystems/EquipmentSubsystem.cpp
      - Source/RoastStaffGAS/Public/Character/Player/RSPlayerController.h
      - Source/RoastStaffGAS/Private/Character/Player/RSPlayerController.cpp
      - Source/RoastStaffGAS/Private/UI/LevelUpWeaponSelectWidget.cpp
      - Source/RoastStaffGAS/Private/UI/WeaponSlotWidget.cpp
    created:
      - Content/Data/Weapon/DT_WeaponDamageCurve.uasset
      - Source/RoastStaffGAS/Public/UI/WeaponReplaceWidget.h
      - Source/RoastStaffGAS/Private/UI/WeaponReplaceWidget.cpp
      - Content/UI/Levelup/WBP_WeaponReplaceUI.uasset
      - _Design/Plans/active/PLAN_WeaponUpgrade_Replace_v1.0.md
    deleted: []
  summary: "무기 강화 로직 + 교체 UI — CurveTable 기반 Lv2/Lv3 데미지 스케일링, EquipmentSubsystem 강화 판정 내재화"
  status: COMMITTED
  bugs_found: []
  bugs_fixed: []

- date: 2026-03-28
  plan: PLAN_EXP_LevelUp_UI_v1.0
  commit: "82444aa / e7a5cb6 / 501862b"
  files:
    modified:
      - Source/RoastStaffGAS/Public/Subsystems/LevelUpSubsystem.h
      - Source/RoastStaffGAS/Private/Subsystems/LevelUpSubsystem.cpp
      - Source/RoastStaffGAS/Private/Subsystems/StageManagerSubsystem.cpp
      - Source/RoastStaffGAS/Public/UI/Player/PlayerStatusBarWidget.h
      - Source/RoastStaffGAS/Private/UI/Player/PlayerStatusBarWidget.cpp
      - Content/UI/Player/WBP_PlayerStatusBar.uasset
      - Content/UI/WBP_HUD.uasset
    created:
      - _Design/Plans/active/PLAN_EXP_LevelUp_UI_v1.0.md
    deleted: []
  summary: "에너미 사망 → 스테이지 시스템 경유 EXP 전달 + PlayerStatusBarWidget EXP 바 Lerp 표시"
  status: COMMITTED
  bugs_found: []
  bugs_fixed: []

- date: 2026-03-28
  plan: PLAN_PlayerHPBarWidget_v1.0
  commit: "714cff4"
  files:
    modified: []
    created:
      - Source/RoastStaffGAS/Public/UI/Player/PlayerHPBarWidget.h
      - Source/RoastStaffGAS/Private/UI/Player/PlayerHPBarWidget.cpp
    deleted: []
  summary: "PlayerHPBarWidget 구현 — ASC HP 감지, Ghost/Health 보간, LowHealth/HitShake 애니메이션"
  status: COMMITTED
  bugs_found: []
  bugs_fixed: []

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
