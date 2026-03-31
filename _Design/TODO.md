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

## ✓ COMMITTED [FEATURE] 게임플로우 인프라 | PLAN_GameFlow_Infra_v1.0
> 커밋: 2e8a443/3feef02/389e200/f05f3c9/df16494/8150d93 | 2026-03-31

  ### ✓ DONE 2026-03-31 [MODULE-1] UITypes.h — EUILayer 4레이어 + EUIID enum
  신규: Source/RoastStaffGAS/Public/Data/EnumUITypes.h
  수정: RSBaseWidget.h
    - [x] EnumUITypes.h 신규 생성, EUILayer 이전 (RSBaseWidget.h에서 제거)
    - [x] EUILayer 4개값: PERSISTENT, PAGE, POPUP, SYSTEM
    - [x] EUIID enum 정의 (NONE~EXIT 전체 목록)
    - [x] RSBaseWidget.h — EUILayer 인라인 제거, EnumUITypes.h include 추가

  ### ✓ DONE 2026-03-31 [MODULE-2] UIManagerSettings — DeveloperSettings
  신규: Public/Systems/UIManagerSettings.h, Private/Systems/UIManagerSettings.cpp
    - [x] UIManagerSettings.h: UDeveloperSettings 파생, Config=Game
    - [x] UIClassMap: TMap<EUIID, TSoftClassPtr<URSBaseWidget>>
    - [x] UILayerMap: TMap<EUIID, EUILayer>
    - [x] static Get() 헬퍼 + GetCategoryName/SectionName

  ### ✓ DONE 2026-03-31 [MODULE-3] UIManagerSubsystem 4레이어 확장
  수정: UIManagerSubsystem.h/.cpp
    - [x] OpenUIByID(EUIID), GetOrCreateWidgetByID(EUIID) 추가
    - [x] PageUIStack, SystemUIStack, UIHistory UPROPERTY 추가
    - [x] PAGE 열기: 기존 PAGE 닫기 → Push + UIHistory Push
    - [x] ClearUIHistory(), BackPage(), SwitchPageUI(EUIID) 추가
    - [x] SYSTEM 레이어 처리, CalculateZOrder/NotifyInputModeChange 수정
    - [x] ResetAllUIStates()에 Page/System/History Clear 포함
    - [x] 기존 OpenUI<T>() 하위 호환 완전 유지

  ### ✓ DONE 2026-03-31 [MODULE-4] RSGameInstance + MapSettings
  신규: Public/Core/RSGameInstance.h, Private/Core/RSGameInstance.cpp
        Public/Systems/MapSettings.h, Private/Systems/MapSettings.cpp
    - [x] ELevelName enum: EnumUITypes.h에 추가 (INTRO, TRANSITION, OUTGAME, STAGE)
    - [x] MapSettings: UDeveloperSettings, TMap<ELevelName, TSoftObjectPtr<UWorld>>
    - [x] RSGameInstance: SetNextLevelName/GetNextLevelName, SetNextStageID/GetNextStageID
    - [x] OpenNextLevelByName(ELevelName): ResetAllUIStates + 0.1s → TRANSITION
    - [x] OpenNextLevelLatent(): MapSettings 조회 → OpenLevel
    - [x] OpenNextStage(FName): SetNextStageID + OpenNextLevelByName(STAGE)
    - [x] DefaultEngine.ini GameInstanceClass 설정
    - [~] StageManagerSubsystem::LoadStage() 호출 — 미구현. DEFERRED (StageManagerSys 완성 후)

---

## [FEATURE] 게임플로우 레벨 | PLAN_GameFlow_Levels_v1.0
> 시작: 2026-04-01 | 기획서: 게임 플로우 아키텍처 기획 v1.0.md

  ### ✓ COMMITTED 2026-04-01 [MODULE-1] IntroController + IntroGameMode (6c2c881)
    - [x] RSIntroGameMode, RSIntroPlayerController 신규
    - [x] OpenFirstWidget(): BACKGROUND + INTRO, OpenTitleScreen(), OnStartGameClicked()
    - [x] RSIntroWidget: FadeAnim → OnTitleOpenRequestedDel 브로드캐스트
    - [x] RSTitleWidget: Btn_Start → OnStartGameRequestedDel → OpenNextLevelByName(OUTGAME)

  ### ✓ COMMITTED 2026-04-01 [MODULE-2] TransitionController + TransitionGameMode (553dc01)
    - [x] RSTransitionGameMode, RSTransitionPlayerController 신규
    - [x] PreloadAssetsAsync stub + FakeProgress FInterpTo(0→0.9) + StartLevelStreaming
    - [x] RSLoadingWidget: ProgressBar + Gear/Icon 회전 타이머 애니메이션
    - [~] RuntimeDataSubsystem::GatherPreloadAssets 연동 — DEFERRED (PLAN_GameFlow_Data MODULE-2)

  ### [MODULE-3] OutGameController + OutGameMode                           [P1]
    - [ ] RSOutGameController, RSOutGameMode 신규                          [P1]
    - [ ] OpenFirstWidget(): OUTGAME(PERSISTENT) + LOBBY(PAGE)             [P1]
    - [ ] 캐릭터/스테이지/설정 버튼 핸들러                                 [P1]

  ### [MODULE-4] 캐릭터 DataTable 스키마                                   [P1]
    - [ ] CharacterDataStructs.h: FCharacterStaticData (DefaultWeaponID 포함)  [P1]
    - [ ] DT_CharacterStatic 에셋 생성                                     [P1]

  ### [MODULE-5] 캐릭터 선택 UI + 스테이지 선택 UI                        [P1]
    - [ ] RSCharacterSelectWidget, RSStageSelectWidget 신규                [P1]
    - [ ] 델리게이트 브로드캐스트 → OutGameController 연결                 [P1]

---

## [FEATURE] 게임플로우 데이터 | PLAN_GameFlow_Data_v1.0
> 시작: 미정 (Levels 완료 후) | 기획서: 게임 플로우 아키텍처 기획 v1.0.md

  ### [MODULE-1] SaveGameSubsystem                                         [P2]
    - [ ] RSGameSave.h + SaveGameSubsystem 신규 (LastSelectedCharacterID + SettingsData)  [P2]

  ### [MODULE-2] RuntimeDataSubsystem                                      [P2]
    - [ ] RuntimeDataSubsystem 신규 (SSOT, HandleSaveGameLoaded, GatherPreloadAssets)  [P2]
    - [ ] TransitionGameMode stub → RuntimeDS 실제 연동                    [P2]

  ### [MODULE-3] DefaultWeapon 자동 장착                                   [P2]
    - [ ] RSGameMode::InitDefaultWeapon() — RuntimeDS → EquipmentComponent  [P2]

  ### [MODULE-4] 인게임 UI EUIID 마이그레이션                              [P2]
    - [ ] RSPlayerController OpenUI<T> → OpenUIByID(EUIID) 전면 교체      [P2]
    - [ ] TSubclassOf 프로퍼티 제거, UIManagerSettings로 이전              [P2]



---

## DEFERRED
<!-- "나중에" 항목. 이유+우선순위 필수 -->
[~] Txt_PlayerName 업데이트 로직 — 캐릭터 이름 시스템 미구현, BP에서만 텍스트 지정 | [P2] | REF: PLAN_PlayerHPBarWidget_v1.0
[~] 진화 시스템 (Evolution/Combination) — DT_Combination + 조합 체크 로직. 강화 시스템 완성 후 착수. | [P2] | REF: PLAN_WeaponUpgrade_Replace_v1.0
[~] 연속 레벨업 시 교체 UI 중첩 처리 — 기획서 미정의, 교체 UI 완성 후 별도 설계 필요 | [P2] | REF: PLAN_WeaponUpgrade_Replace_v1.0
[x] WeaponSlotWidget 강화 후 SkillIcon visibility 버그 수정 | 07153a4 | 2026-03-30
[~] [!] SR HIGH#2 — OnWeaponSlotFull 교체UI 오픈 시 TimeDilation = 0.f 미적용. PLAN MODULE-7 SetPause 스펙 불일치. RSPlayerController.cpp:242 수정 필요 | [P1] | REF: SR_20260330_WeaponUpgradeReplace

---

## COMPLETED_LOG
<!-- compact 형식: [x] FEATURE명 | 커밋 | 날짜 | 플랜파일 -->
[x] 무기강화+교체UI(CurveTable) | b6b18d4,ed1513a,e35d380,0587332,07153a4,aa6d657 | 2026-03-30 | PLAN_WeaponUpgrade_Replace_v1.0
[x] EXP전달-레벨업UI | 82444aa,e7a5cb6 | 2026-03-28 | PLAN_EXP_LevelUp_UI_v1.0
[x] 플레이어 HP바 위젯 | 714cff4 | 2026-03-28 | PLAN_PlayerHPBarWidget_v1.0
[x] 수동 발사 클릭 미인식 버그 픽스 | 3e21a68 | 2026-03-27 | PLAN_ManualFireBugFix_v1.0
