# RoastStaffGAS

> UE5 C++ · Gameplay Ability System 기반 쿼터뷰 디펜스 RPG 로그라이크

---

## 한 줄 아키텍처 설명

> "CSV → DataTable → GameDataSubsystem 단일 창구로 모든 정적 데이터를 관리하고, 스테이지 진행·웨이브·풀링·UI·저장을 각각 독립 서브시스템으로 분리해 단일 책임을 지켰습니다. 플레이어는 PlayerState-ASC, 에너미는 Self-ASC + IPoolableInterface로 풀 재활용을 구현했으며, Intro → Transition → OutGame → InGame 씬 전환을 GameMode 단위로 격리해 씬 간 결합도를 최소화했습니다."

---

## 기술 스택

| 항목 | 내용                                |
|------|-----------------------------------|
| 엔진 | Unreal Engine 5 (C++)             |
| 전투 시스템 | Gameplay Ability System (GAS)     |
| 데이터 | Data-Driven - DataTable / CSV 임포트 |
| AI | Behavior Tree + AIController      |
| 플랫폼 | 싱글플레이어 전용 (Replication 비활성)       |

---

## 핵심 아키텍처

### 데이터 파이프라인

```
CSV (외부 원본)
  └─► DataTable (UE 에셋)
        └─► GameDataSubsystem (GDS)   ← 단일 조회 창구
              └─► 각 Subsystem / GA 에서 GetXxx() 호출
```

**GameDataSubsystem (GDS)** 는 `UGameInstanceSubsystem`으로 씬 전환과 무관하게 유지됩니다.
모든 정적 데이터(캐릭터·무기·스킬·에너미·스테이지·웨이브)는 반드시 GDS를 통해서만 조회합니다.
조회 실패는 `bool + OutParam` 패턴으로 반환하고, 이후 처리는 요청 시스템의 책임입니다.

---

### 서브시스템 구조

```
GameInstance 스코프 (씬 간 유지)
├── GameDataSubsystem  (GDS) — 정적 DataTable 로드/캐싱/조회
├── RuntimeDataSubsystem (RDS) — 세션 내 동적 상태 (해금, 슬롯, 선택 캐릭터)
├── SaveGameSubsystem   (SGS) — 영구 저장 데이터
├── UIManagerSubsystem  (UMS) — 위젯 생명주기 · 4레이어 관리
└── EquipmentSubsystem        — 무기 장착/슬롯 관리

World 스코프 (스테이지 단위)
├── StageManagerSubsystem     — 웨이브 진행 · 에너미 처치 집계
└── PoolingSubsystem          — Actor/Widget 오브젝트 풀 + 비동기 프리웜
```

#### UIManagerSubsystem — 4레이어 위젯 관리

| 레이어 | 용도 | ZOrder |
|--------|------|--------|
| PERSISTENT | HUD 등 항상 표시 | 100 |
| PAGE | 메인 콘텐츠 (동시 1개, 히스토리 BackPage 지원) | 200 |
| POPUP | 모달 팝업 (스택 중첩 가능) | 300+ |
| SYSTEM | 종료 확인 / 에러 (최상위) | 500 |

위젯은 클래스 직접 참조 대신 `EUIID` 열거형 ID로 열고 닫습니다.

---

### 캐릭터 계층 및 GAS 구조

```
ABaseCharacter (IAbilitySystemInterface)
│  공통: GAS 초기화 계약, 속성 델리게이트, FloatingDamageWidget 구독
│
├── ARSPlayerCharacter
│     ASC 위치: ARSPlayerState   (PlayerState-ASC 패턴)
│     컴포넌트: SpringArm · Camera · EquipmentComponent
│
└── AEnemyBaseCharacter  (+ IPoolableInterface)
      ASC 위치: Self             (Enemy-ASC 패턴)
      풀 생명주기: OnPoolActivate / OnPoolDeactivate
      초기화: InitializeEnemy(EnemyID) — GDS에서 스탯 주입
      │
      ├── MeleeEnemy
      ├── RangedEnemy
      ├── EliteEnemy
      └── BossEnemy  (Phase1 / Phase2 전환)
```

#### AttributeSet

```
BaseAttributeSet
├── PlayerAttributeSet  — HP · MP · EXP · Level · 방어력 · 이동속도
└── EnemyAttributeSet   — HP · 공격력 (최소 세트)
```

#### Gameplay Ability

```
GA_Base
├── GA_ProjectileAttack  — GDS 조회 → 투사체 풀 획득 → 발사
└── GA_SummonBase        — GDS 조회 → 소환물 풀 획득 → 배치
```

---

### 오브젝트 풀링

`PoolingSubsystem`은 Actor 풀과 Widget 풀을 모두 관리합니다.

- **비동기 프리웜**: `RequestAsyncPreWarm()` — 스테이지 로딩 중 백그라운드 생성
- **인터페이스 기반**: `IPoolableInterface` — `OnPoolActivate / OnPoolDeactivate`
- **GC 안전**: `FActorPoolBucket` / `FWidgetPoolBucket` USTRUCT 래퍼로 UPROPERTY 추적

```
GameMode::BeginPlay
  └─► PoolingSubsystem::RequestAsyncPreWarm(PreWarmList)
        └─► OnPreWarmCompleted ─► StartStageFlow
```

---

### 게임 플로우 및 씬 전환

```
RSIntroGameMode           — 타이틀 화면
  └─► RSTransitionGameMode  — 에셋 비동기 프리로드 (캐릭터 Mesh/AnimBP, 에너미 BT)
        └─► RSOutGameMode    — 로비 · 캐릭터 선택 · 스테이지 맵 선택
              └─► RSGameMode   — 인게임 (스테이지 조율자)
```

각 씬은 독립된 GameMode + PlayerController 쌍을 사용합니다.
씬 데이터를 `RuntimeDataSubsystem`으로 전달해 씬 간 직접 참조를 제거했습니다.

#### InGame 초기화 순서 (ARSGameMode)

```
BeginPlay
  ├─ InitializePlayer(CharID)      — GDS 조회 → ASC + 기본 무기 설정
  ├─ InitializeStage()             — StageManagerSubsystem 초기화
  └─ InitializePreWarm(Spawner)    — PoolingSubsystem 프리웜 시작
        └─ OnPreWarmCompleted()     — 입력 복구 + LoadingUI 닫기 + StartStageFlow
```

---

### AI 시스템

```
EnemyAIController
  └─► BehaviorTree
        ├── BTTask_FireProjectile    — 풀에서 투사체 획득 → 발사
        ├── BTTask_MeleeCharge       — 돌진 이동
        ├── BTTask_RangedReposition  — 원거리 재배치
        ├── BTTask_ExecuteShockwave  — 광역 충격파 (보스)
        ├── BTDecorator_IsPhase2     — 보스 HP 50% 미만 판정
        ├── BTDecorator_ShockwaveReady — 쿨타임 판정
        └── BTDecorator_RandomChance   — 가중치 랜덤 분기
```

---

## 주요 설계 결정 및 이유

| 결정 | 이유 |
|------|------|
| GDS 단일 창구 | 하드코딩 제거, 데이터 변경 시 CSV만 수정 |
| Enemy-Self ASC | 다수 에너미 인스턴싱 비용 최소화, PlayerState 불필요 |
| IPoolableInterface | 스폰/제거 비용 제거, 생명주기 명확화 |
| UIManagerSubsystem ID 기반 API | 위젯 클래스 직접 참조 없이 레이어 정책 중앙화 |
| StageManager vs GameMode 분리 | GameMode는 초기화 조율만, 웨이브 로직은 WorldSubsystem에 위임 |
| 씬별 GameMode/PC 쌍 | 씬 간 책임 격리, 불필요한 크로스-씬 참조 차단 |

---

## 폴더 구조

```
Source/RoastStaffGAS/
├── Public/
│   ├── Core/          — GameMode, GameInstance, PlayerController (씬별)
│   ├── Character/     — BaseCharacter, Player, Enemy 계층
│   ├── GAS/           — AttributeSet, Abilities, GameplayTags
│   ├── Subsystems/    — GDS, RDS, SGS, UMS, Stage, Pooling, Equipment, LevelUp
│   ├── AI/            — BTTask, BTDecorator
│   ├── UI/            — 씬별 위젯 (InGame, OutGame, Intro, Transition, Enemy, Player)
│   ├── Objects/       — Projectile, Summon
│   ├── Component/     — EquipmentComponent
│   ├── Interface/     — IPoolableInterface
│   ├── Data/          — DataTableStructs, RuntimeDataStructs, EnumTypes
│   └── System/        — EnemySpawner, UIManagerSettings, MapSettings
└── Private/           — 구현부 (헤더와 동일 구조)

_Design/
├── References/Systems/ — 시스템별 기획서
├── Plans/              — PLAN 계획서 (active / completed)
├── Reviews/            — 시니어 리뷰 결과
└── Learning/           — 학습 리포트
```
