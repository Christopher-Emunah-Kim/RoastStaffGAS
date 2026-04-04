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

## ✓ COMMITTED [FEATURE] 게임플로우 레벨 | PLAN_GameFlow_Levels_v1.0
> 커밋: c0be61f / 2118448 | 2026-04-01.md

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

  ### ✓ DONE 2026-04-01 [MODULE-3] OutGameController + OutGameMode
    - [x] RSOutGameMode: AGameModeBase 파생, DefaultPawnClass=None, PlayerControllerClass=RSOutGamePlayerController
    - [x] RSOutGamePlayerController::BeginPlay() → SetShowMouseCursor(true) + SetInputMode(UIOnly) + OpenFirstWidget()
    - [x] OpenFirstWidget(): OUTGAME(PERSISTENT), ClearUIHistory(), LOBBY(PAGE) 순서 오픈
    - [x] OnCharacterSelectClicked() UFUNCTION: UIManager::SwitchPageUI(EUIID::CHAR_SELECT)
    - [x] OnStageSelectClicked() UFUNCTION: UIManager::SwitchPageUI(EUIID::STAGE_SELECT)
    - [x] OnSettingClicked() UFUNCTION: UIManager::OpenUIByID(EUIID::SETTING)
    - [x] OnCharacterSelected(FName CharID) UFUNCTION: RuntimeDS stub 주석 + UIManager::BackPage()
    - [x] OnStageSelected(FName StageID) UFUNCTION: GI->OpenNextStage(StageID)
    - [~] CharacterSelectWidget, StageSelectWidget 델리게이트 구독 — MODULE-5(RSCharacterSelectWidget/RSStageSelectWidget) 구현 후 연결 | REF: MODULE-5
    - [x] OUTGAME 레벨 World Settings → RSOutGameMode 지정 [에디터 완료]

  ### ✓ DONE 2026-04-01 [MODULE-4] 캐릭터 DataTable 스키마
    - [x] FCharacterStaticData: DataTableStructs.h에 직접 추가 (CharacterID~BaseAttackPower, USTRUCT 기본값 전체)
    - [x] GameDataConfig.h: CharacterStaticTable 추가
    - [x] GameDataSubsystem: LoadedCharacterTable + CharacterCache + GetCharacterStaticData stub
    - [x] DT_CharacterStatic 에셋 생성 (에디터, Content/Data/Character/DT_CharacterStatic)

  ### [~] [MODULE-5] 캐릭터 선택 UI + 스테이지 선택 UI — PLAN_OutGame_SelectUI_v1.0으로 대체·확장 | REF: PLAN_OutGame_SelectUI_v1.0

---

## ✓ COMMITTED [FEATURE] OutGame 선택 UI | PLAN_OutGame_SelectUI_v1.0
> 커밋: 1a21f8c~8d68391 (11개) + 2944f16/71f1ac0/1ed214f/15ac6d4 | 2026-04-01~02

  ### ✓ DONE 2026-04-01 [MODULE-1] DataSchema 확장
  수정: DataTableStructs.h, EnumUITypes.h, EnumTypes.h, RSGameInstance.cpp
    - [x] EnumTypes.h: ECharacterGrade(SSR/SR/R/N) 추가
    - [x] EnumTypes.h: ECharacterUnlockType(DEFAULT/STAGE_CLEAR/CURRENCY) 추가
    - [x] EnumUITypes.h: EUIID::CHAR_GRID_POPUP 추가
    - [x] EnumUITypes.h: ELevelName::STAGE → STAGE_1 rename + STAGE_2 추가
    - [x] RSGameInstance.cpp: ELevelName::STAGE → STAGE_1 수정 (TODO: WorldLevel 조회 로직 MODULE-5에서)
    - [x] DataTableStructs.h: FCharacterStaticData Grade/UnlockType/UnlockStageID/UnlockCost 추가
    - [x] DataTableStructs.h: FStageStaticData DisplayName/NextStageID/bIsBoss/UnlockStageID/WorldLevel/Thumbnail 추가

  ### ✓ DONE 2026-04-01 [MODULE-2] SaveGameSubsystem
  신규: RSGameSave.h, SaveGameSubsystem.h/.cpp
    - [x] URSSaveGame: UnlockedCharIDs/ClearedStageIDs/LastSelectedCharID/SettingsData/SaveVersion
    - [x] FRSSettingsData USTRUCT (볼륨 3종 기본값 1.0f)
    - [x] SaveGameSubsystem: Initialize/Deinit/SaveGame/LoadGame/Is*/Add*/Get/Set API 전체
    - [x] OnSaveGameLoadedDel 델리게이트
    - [x] DEFERRED TODO 주석 (FStageRecord/FTransactionState 등)

  ### ✓ DONE 2026-04-01 [MODULE-3] RSCharacterSelectWidget + RSCharacterGridPopupWidget
  신규: Public/UI/OutGame/ RSCharacterSelectWidget.h/.cpp, RSCharacterGridPopupWidget.h/.cpp
        Public/UI/OutGame/ RSCharacterEntryWidget.cpp (구현)
    - [x] RSCharacterSelectWidget: 캐러셀 PAGE, FOnCharacterSelectedDel, PopulateCarousel [P0]
    - [x] RSCharacterGridPopupWidget: 그리드 POPUP, 정렬(해금→Grade→Level stub) [P0]

  ### [~] [MODULE-3c] CharacterSelectWidget Img_Portrait 정보 패널 — 전체 구현 완료 후 | [P2]
  수정: RSCharacterSelectWidget.h/.cpp
    - [~] Img_Portrait BindWidget 추가 + UpdateInfoPanel에서 TSoftObjectPtr<UTexture2D> 비동기 로드 + SetBrushFromTexture [P2]

  ### ✓ DONE 2026-04-01 [MODULE-3b] RSLobbyWidget
  신규: Public/UI/OutGame/RSLobbyWidget.h/.cpp
    - [x] Btn_CharacterSelect/StageSelect/Settings BindWidget + NativeOnInitialized 바인딩
    - [~] WBP_CharacterSelect 레이아웃 디자인 — 다음 세션에서 진행 [P1] | REF: MODULE-3 TODO

  ### ✓ DONE 2026-04-02 [MODULE-0] LobbyWidget + CharacterSelectWidget 플로우 수정
  수정: RSLobbyWidget.h/.cpp, RSCharacterSelectWidget.h/.cpp
    - [x] RSLobbyWidget: Btn_StageSelect BindWidget 제거, OnStageSelectClicked 제거
    - [x] RSCharacterSelectWidget: FOnStageSelectRequested 델리게이트 추가
    - [x] RSCharacterSelectWidget: NativeConstruct — Btn_StageSelect.SetIsEnabled(false)
    - [x] RSCharacterSelectWidget: OnCharacterEntryClicked — Btn_StageSelect 활성화
    - [x] RSCharacterSelectWidget: OnStageSelectClicked — NAME_None 가드 + Broadcast

  ### ✓ DONE 2026-04-02 [MODULE-4] RSStageNodeWidget + RSStageSelectWidget (신규)
  신규: Public/UI/OutGame/ RSStageNodeWidget.h/.cpp, RSStageSelectWidget.h/.cpp
  수정: Public/Data/EnumTypes.h (EStageNodeState 추가)
    - [x] EnumTypes.h: EStageNodeState { AVAILABLE, CLEARED, LOCKED } 추가
    - [x] RSStageNodeWidget.h: URSBaseWidget 파생, BindWidget 6종 (Btn_Node, Img_Thumbnail, Img_BossIcon, Img_LockIcon, Img_ClearMark, Txt_NodeName), FOnStageNodeClickedDel, SetNodeState()
    - [x] RSStageNodeWidget.cpp: NativeOnInitialized Btn_Node 바인딩, SetNodeState 상태별 Visibility + SetIsEnabled
    - [x] RSStageSelectWidget.h: URSBaseWidget 파생, FOnStageSelectedDel, BindWidget 7종 (Btn_Back, NodeMapContainer, Txt_SelectedName, Txt_TimeLimit, Txt_NodeStatus, Img_SelectedThumb, Btn_Confirm), TSubclassOf<URSStageNodeWidget> NodeWidgetClass, PopulateNodeMap()
    - [x] RSStageSelectWidget.cpp: NativeOnInitialized — Btn_Back/Btn_Confirm 바인딩
    - [x] RSStageSelectWidget.cpp: NativeConstruct — 미선택 초기화 + PopulateNodeMap()
    - [x] RSStageSelectWidget.cpp: PopulateNodeMap() — GDS 조회 + SGS 기반 EStageNodeState 판정 + 동적 생성
    - [x] RSStageSelectWidget.cpp: OnNodeClicked — 상세 패널 갱신, Btn_Confirm 활성 제어
    - [x] RSStageSelectWidget.cpp: OnConfirmClicked — NAME_None 가드 + OnStageSelectedDel.Broadcast
    - [x] RSStageSelectWidget.cpp: OnBackClicked — UMS::BackPage()
    - [x] RSStageSelectWidget.cpp: NativeDestruct — NodeWidget RemoveDynamic 루프

  ### ✓ DONE 2026-04-02 [MODULE-5] OGPC 델리게이트 바인딩 완성
  수정: RSOutGamePlayerController.h/.cpp
    - [x] CachedCharSelectWidget / CachedStageSelectWidget UPROPERTY() 추가 (GC 추적)
    - [x] EndPlay() override 선언 + RemoveDynamic 해제 구현
    - [x] OpenFirstWidget: GetOrCreateWidgetByID(CHAR_SELECT/STAGE_SELECT) 미리 생성 + 델리게이트 일괄 바인딩
    - [x] OnStageSelectClicked: SwitchPageUI(STAGE_SELECT) — CharacterSelectWidget::OnStageSelectRequestedDel 수신
    - [x] OnCharacterSelected: SGS::SetLastSelectedCharacter (BackPage 제거)
    - [x] OnStageSelected: SGS::GetLastSelectedCharacter NAME_None 가드 + SaveGame() + OpenNextStage
    - [x] OnCharacterSelectClicked 복원 — LobbyWidget이 OGPC 경유하도록 변경
    - [x] UIManagerSubsystem::SwitchPageUI void → URSBaseWidget* 반환 타입 변경
    - [x] RSLobbyWidget::OnCharacterSelectClicked → OGPC::OnCharacterSelectClicked 경유

  ### ✓ DONE 2026-04-01 [MODULE-6] GDS GetAll API
  수정: GameDataSubsystem.h/.cpp
    - [x] GetAllCharacterStaticData(TArray<FCharacterStaticData>&) 추가
    - [x] GetAllStageStaticData(TArray<FStageStaticData>&) 추가

---

## ✓ COMMITTED [FEATURE] 게임플로우 데이터 | PLAN_GameFlow_Data_v1.0
> 커밋: d622281 / f10dab1 | 2026-04-02

  ### [MODULE-1] SaveGameSubsystem                                         [P2]
    - [x] RSGameSave.h + SaveGameSubsystem 신규 (LastSelectedCharacterID + SettingsData) — SelectUI MODULE-2에서 완료

  ### ✓ DONE 2026-04-02 [MODULE-2] RSGameMode 스테이지+캐릭터 적용
    - [x] RSGameMode::BeginPlay — DefaultStageID → GI::GetNextStageID() 교체
    - [x] RSGameMode::BeginPlay — SGS::GetLastSelectedCharacter() 조회 + 캐릭터 스탯 초기화
    - [x] RSPlayerState::ApplyCharacterStats(FName CharID) — GDS 조회 후 전체 스탯(9개 어트리뷰트) 적용
    - [x] RSPlayerState::ApplyBaseStats() 제거 — 하드코딩 완전 제거
    - [x] FCharacterStaticData — BaseDEF/BaseAttackSpeed/BaseCastingSpeed/BaseCriticalRate/BaseCriticalDamage 추가
    - [x] DT_CharacterStatic 에디터에서 신규 컬럼(5개) 캐릭터별 값 입력 [에디터 완료]

  ### ✓ DONE 2026-04-02 [MODULE-3] DefaultWeapon 자동 장착
    - [x] RSGameMode::InitDefaultWeapon() — GDS::GetCharacterStaticData(CharID).DefaultWeaponID → EquipmentSubsystem::EquipWeapon

  ### ✓ DONE 2026-04-02 [MODULE-4] RuntimeDataSubsystem
  신규: RuntimeDataSubsystem.h/.cpp
  수정: RuntimeDataStructs.h, GameDataSubsystem.h/.cpp, SaveGameSubsystem.h/.cpp
        RSTransitionGameMode.h/.cpp, RSOutGamePlayerController.cpp
    - [x] FCharacterPreloadBundle / FEnemyPreloadBundle: RuntimeDataStructs.h 추가
    - [x] GDS: GetCharacterPreloadBundle / GetEnemyPreloadBundle 추가
    - [x] SGS: GetSettingsData / UpdateSettingsData 추가
    - [x] URuntimeDataSubsystem: SelectedCharacterID/SettingsData SSOT, SGS 델리게이트 구독
    - [x] RSTransitionGameMode: stub 제거 → RDS::GatherPreloadAssets + FStreamableManager 비동기 로드
    - [x] RSOutGamePlayerController: OnCharacterSelected SGS→RDS, OnStageSelected SerializeToPersistentData 경유

  ### ✓ COMMITTED 783b4e7 2026-04-03 [MODULE-5] 인게임 UI EUIID 마이그레이션
  수정: RSPlayerController.h/.cpp
    - [x] RSPlayerController OpenUI<T> → OpenUIByID(EUIID) 전면 교체
    - [x] TSubclassOf 프로퍼티 제거 (HUDWidgetClass/LevelUpUIClass/WeaponReplaceUIClass), UIManagerSettings로 이전
    - [x] CachedHUDUI/CachedLevelUpWidget/CachedWeaponReplaceWidget 제거 — UIManagerSubsystem CachedWidgetsByID로 일원화
    - [x] BP RSPlayerController에서 기존 위젯 클래스 프로퍼티 제거 + UIManagerSettings에서 설정 안내 [에디터 작업]



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
[x] 무기강화+교체UI(CurveTable) | b6b18d4,ed1513a,e35d380,0587332,07153a4,aa6d657 | 2026-03-30 | PLAN_WeaponUpgrade_Replace_v1.0
[x] EXP전달-레벨업UI | 82444aa,e7a5cb6 | 2026-03-28 | PLAN_EXP_LevelUp_UI_v1.0
[x] 플레이어 HP바 위젯 | 714cff4 | 2026-03-28 | PLAN_PlayerHPBarWidget_v1.0
[x] 수동 발사 클릭 미인식 버그 픽스 | 3e21a68 | 2026-03-27 | PLAN_ManualFireBugFix_v1.0
