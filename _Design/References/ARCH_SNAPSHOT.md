# ARCH_SNAPSHOT
> 현재 구현 상태 스냅샷. 기획서(무엇을 만들 것인가)가 아닌 현실(지금 어떻게 되어있는가).
> INIT마다 로드. CODE 완료 시 [E] 단계에서 갱신.
> COMPACT 트리거: DESIGN_DECISIONS 10개 초과 / INTEGRATION_MAP 15개 초과 / "ARCH 정리해줘"
> 상세 compact 정책: .claude/refs/protocols.md#ARCH_COMPACT

---

## SYSTEM_MAP
```
[DataTable Layer]
  CSV → DataTable(DT_*) → Subsystem → GA  ← 핵심 아키텍처 원칙

[GameMode]  ARSGameMode
  └─ BeginPlay → PoolingSubsystem::InitializePool  (임시 — Stage시스템 이관 예정)

[Player Layer]
  ARSPlayerController          ← 입력 + UI 전담 (별도 서브시스템 없음)
    ├─ Enhanced Input → 에임 좌표 캐시
    └─ UI 관리 (SetInputMode + SetShowMouseCursor 쌍)
  ARSPlayerCharacter           ← 캐릭터 상태 전담
    ├─ 카메라 / SpringArm
    ├─ UEquipmentComponent     ← 장비 데이터만 (UI 없음)
    ├─ GAS 초기화
    └─ HandleDeath

[GAS Layer]
  AbilitySystemComponent (ASC) ← 소유권 확인 필수
  GameplayAbility (GA)
  GameplayEffect (GE)

[Pooling Layer]
  UPoolingSubsystem
    └─ PreWarm / Activate / Deactivate 흐름

[Logging]
  KHS_* 매크로 전용 (UE_LOG 금지)
  KHS_TRACE / KHS_DEBUG / KHS_INFO / KHS_WARN / KHS_ERROR / KHS_FATAL
```

---

## CLASS_REGISTRY
> 확정된 책임만. 추측 기재 금지. 변경 시 DESIGN_DECISIONS에 결정 추가 후 수정.

| 클래스 | 핵심 책임 | 명시적 비책임 |
|--------|----------|--------------|
| ARSGameMode | 게임 흐름 제어, InitializePool 호출(임시) | - |
| ARSPlayerController | Enhanced Input 처리, 에임 좌표 캐시, UI 관리(SetInputMode/Cursor) | 캐릭터 물리, 장비 관리 |
| ARSPlayerCharacter | 카메라/SpringArm, EquipmentComponent 소유, GAS 초기화, HandleDeath | 입력 처리, UI 관리 |
| UEquipmentComponent | 장비 데이터 보유 | UI 갱신, 입력 처리 |
| UPoolingSubsystem | 오브젝트 풀 생성·관리·회수 | 게임 로직 |

---

## DESIGN_DECISIONS
> 결정 + 이유 + 변경 조건. 10개 초과 시 compact 검토.

| # | 결정 | 이유 | 변경 조건 |
|---|------|------|----------|
| D1 | ARSPlayerController가 입력+UI 모두 담당 | 단일 제어점. 별도 UIManager/PlayerOperationSubsystem 불필요 | 멀티플레이어 도입 시 재검토 |
| D2 | InitializePool → GameMode::BeginPlay (임시) | Stage 시스템 미구현 상태의 임시 위치 | Stage 시스템 구현 시 이관 |
| D3 | 클래스 할당 BP / 이벤트·델리게이트 바인딩 C++ | BP 유연성 + C++ 타입 안전성 분리 | 원칙 재논의 시만 |
| D4 | SetGamePaused: UIManager→PC→SetPause 체인 | GameMode 직접 호출 시 UI 상태 불일치 발생 | - |
| D5 | 게임 정체성 전환: 캐릭터 빌드 서바이버 | 무방향 로그라이크 → 캐릭터 고유스킬+메타빌드업+자동무기 구조로 확정 (2026-04-10 설계회의) | - |
| D6 | 무기 자동발사 / 캐릭터 스킬 Q·E 수동발동 | 수동발사 폐기. 무기=자동, 스킬=능동 개입으로 역할 분리 | - |
| D7 | 레벨업 카드: 정적(스탯/패시브)+동적(무기) 혼합, 무기 최소 1장 보장 | 카드 풀이 무기만이면 매 런 동일 — 다양성 확보 | - |
| D8 | 스탯트리 공통 / 스킬레벨은 BIG노드 마일스톤으로 해금 | 별도 재화 소비 없이 트리 진행이 스킬 성장으로 연결 | - |
| D9 | 재화 단일(골드) / 스테이지 결과에서만 획득 | 인게임 골드 UI 불필요, 반복플레이 유도 | - |
| ★ | **상세 설계 기준문서**: `_Design/References/Systems/게임 시스템 개선안 v1.0.md` | 2026-04-10 확정. PLAN/CODE 전 필독 | - |
| SD1 | 무기 슬롯 수 SLOT_COUNT=2 | 3→2 축소. 캐릭터 스킬 슬롯 2개와 역할 분리 | 기획 변경 시만 |
| SD2 | SpawnPreview → SummonPreviewObject 재활용 | 신규 클래스 불필요. GA_CharacterSkill이 ASummonPreviewObject 직접 스폰 | 다형성 필요 시 재검토 |
| SD3 | EWeaponBaseType + FString EvolutionTag 병행 | BaseType 제거 시 기존 BP 참조 전부 교체 비용 큼 | 진화 시스템 본격 착수 시 |
| SD4 | DT_CharacterSkill = 에디터 직접 편집 전용 | TArray<FCharacterSkillLevelData> 중첩 → CSV 임포트 불가 구조 | - |

---

## PATTERNS
> 이 프로젝트의 확립된 코딩 방식. 위반 시 SR 지적 대상.

```
[UMG 위젯]
  AddDynamic → NativeOnInitialized 전용
  NativeConstruct 사용 금지
  이유: 캐시/풀링 재사용 시 중복 바인딩 버그 발생

[Poolable Actor]
  BeginPlay에 OnPoolDeactivate() 호출 필수
  이유: 풀에서 꺼낼 때 초기 비활성 상태 보장

[Input / UI 상태]
  SetInputMode + SetShowMouseCursor 항상 쌍으로 작성
  이유: 한쪽만 변경 시 입력 모드와 커서 상태 불일치

[GAS 버그 진단 순서]
  1. git diff → 최근 변경 파악
  2. 함수 진입 여부 확인 (로그/브레이크포인트) ← 반드시 2번째
  3. 레이어 좁히기 (Input→PC→Character→Subsystem→GAS)
  4. ASC 내부 진단
  이유: 내부 먼저 보면 잘못된 레이어 수정 후 롤백 발생

[DataTable 연동 USTRUCT]
  모든 필드에 최소 동작 보장 기본값 필수
  이유: 기본값 없으면 미입력 행에서 정의되지 않은 동작

[UPROPERTY]
  UObject* 참조는 반드시 UPROPERTY() 부착
  이유: GC 추적 누락 시 허상 포인터
```

---

## INTEGRATION_MAP
> 시스템 연결 지점 (호출자 → 수신자). CODE 완료 시 갱신. 15개 초과 시 compact 검토.
> 형식: `[호출자]::[함수] → [수신자]::[함수]` | 트리거/조건

| 호출자 → 수신자 | 트리거/조건 |
|----------------|------------|
| `UGameDataSubsystem::LoadDataTables()` → `UDataTable* (3종)` | Initialize() 시 GameDataConfig 경로로 로드 |
| `UGameDataSubsystem::CacheAllData()` → `CharacterSkillCache / PassiveCache / LevelUpCardCache` | LoadDataTables() 완료 직후 |
| `외부 시스템::X` → `UGameDataSubsystem::GetCharacterSkillExecData(CharacterID, Slot, Level)` | SkillManagerSubsystem 초기화 시 (M-5) |
| `외부 시스템::X` → `UGameDataSubsystem::GetSkillsByCharacter(CharacterID)` | 캐릭터 선택 후 스킬 목록 조회 시 (M-5) |
| `외부 시스템::X` → `UGameDataSubsystem::GetAllLevelUpCards()` → `LevelUpSubsystem 카드풀 구성` | 레벨업 이벤트 (M-6) |
| `외부 시스템::X` → `UGameDataSubsystem::GetPassiveData(PassiveID)` → `PassiveSlotSubsystem::TryAddPassive` | 패시브 카드 선택 시 (M-7) |

---

## FROZEN
> 변경 금지 목록. 의도적으로 확정된 것만.
> 변경하려면: DESIGN_DECISIONS에 새 결정 추가 → 이 목록 수정.

*(미기록)*
