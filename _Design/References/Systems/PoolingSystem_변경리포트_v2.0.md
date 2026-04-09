# PoolingSystem 변경 리포트 v2.0
> 작성: 2026-04-08 | 커밋: af3c5cd (feat) + 8011f7f (refactor)
> 비교 기준: SPRINT-5 이전(분산 풀링) → SPRINT-5 이후(중앙화 + AsyncPreWarm)

---

## 1. 변경 전 vs 변경 후 구조 비교

### 변경 전 — 분산 풀링

```
EnemySpawner::InitPools()
  ├── PoolingSubsystem->InitializePool(EnemyClass, N)   // 직접 호출
  └── PoolingSubsystem->InitializePool(ProjectileClass, M)

RSPlayerController::BeginPlay()
  └── PoolingSubsystem->InitializeWidgetPool(FloatingDamageClass, K, PC)

EquipmentSubsystem::CommitSlot()
  └── PoolingSubsystem->InitializePool(WeaponProjectileClass, 10)
```

**문제점:**
- 풀 초기화 책임이 EnemySpawner / PC / EquipmentSubsystem에 분산
- 스테이지 시작 시 모든 풀이 동기적으로 스폰 → 첫 프레임 히치
- 로딩 화면과 연동 불가 (진행률 없음)

---

### 변경 후 — PoolingSubsystem 중앙화 + AsyncPreWarm

```
RSGameMode::BeginPlay()
  ├── InitializePlayer()
  ├── InitializeStage()
  └── InitializePreWarm(Spawner)
        ├── BuildPreWarmList()         // 요청 목록 구성
        └── PoolingSubsystem->RequestAsyncPreWarm(List)
              └── OnPreWarmComplete 구독

PoolingSubsystem::Tick() (매 프레임)
  └── TickPreWarm()
        └── SpawnOnePreWarmUnit()      // 프레임당 N개씩 배치 스폰

RSGameMode::Tick()
  └── UpdatePreWarmProgress()
        └── GetLoadingWidget()->SetLoadingProgress(progress)

RSGameMode::OnPreWarmCompleted()       // 델리게이트 수신
  ├── PC->EnableInput()
  ├── GetLoadingWidget()->FinishLoading()
  └── StartStageFlow()
```

---

## 2. API 변경 사항

### PoolingSubsystem — 추가된 API

| 함수 | 설명 |
|------|------|
| `RequestAsyncPreWarm(TArray<FPoolPreWarmRequest>)` | 비동기 프리웜 요청 큐 등록 |
| `GetPreWarmProgress()` → float | 진행률 0.0~1.0 반환 |
| `OnPreWarmComplete` (FSimpleMulticastDelegate) | 완료 시 1회 브로드캐스트 |
| `InitializeWidgetPool(WidgetClass, Count, PC)` | 위젯 풀 즉시 초기화 |
| `SpawnPooledWidget(WidgetClass, PC)` | 위젯 풀에서 꺼내기 |
| `ReturnWidgetToPool(Widget)` | 위젯 반납 + Collapsed |
| `DrainPool(ActorClass)` | 특정 클래스 풀 버킷 완전 제거 |

### PoolingSubsystem — 내부 private 헬퍼 (이번 리팩토링)

| 함수 | 역할 |
|------|------|
| `AddActorToPool(ActorClass)` | TrySpawnActor → 버킷 적재. InitializePool/TickPreWarm 공용 |
| `SpawnOnePreWarmUnit(Req)` | 요청 1단위 처리. false = PC 미준비(루프 중단) |
| `PopFirstValid<T>(Pool)` | 유효 인스턴스 팝. GC 무효화 항목 자동 건너뜀 |
| `TickPreWarm()` | 프레임당 PreWarmBatchSize개 배치 스폰 |

### EnemySpawner — 변경

| 변경 | 내용 |
|------|------|
| `InitPools()` 내 InitializePool 호출 제거 | 풀 초기화 책임 GameMode로 이관 |
| `GetEnemyProjectileClass()` 추가 | GameMode가 BuildPreWarmList에서 조회 |
| `GetProjectilePoolCount()` 추가 | 투사체 풀 수량 getter |
| `GetPoolCountPerClass()` 추가 | 에너미 클래스당 풀 수량 getter |

### RSGameMode — 추가된 private 함수

| 함수 | 역할 |
|------|------|
| `InitializePreWarm(Spawner)` | 프리웜 시작 조율 |
| `BuildPreWarmList(Spawner)` | GDS + Spawner getter로 요청 목록 구성 |
| `CollectUniqueEnemyClasses()` | 웨이브 데이터에서 고유 에너미 클래스 수집 |
| `GetLoadingWidget()` | EUIID::LOADING 위젯 조회 헬퍼 |
| `MakeActorRequest(Class, Count)` | FPoolPreWarmRequest Actor용 생성 헬퍼 |
| `MakeWidgetRequest(Class, Count)` | FPoolPreWarmRequest Widget용 생성 헬퍼 |
| `OnPreWarmCompleted()` | 완료 콜백 — 입력 복구 + 로딩 종료 + 스테이지 시작 |
| `UpdatePreWarmProgress()` | Tick에서 LoadingWidget 진행률 반영 |
| `CloseLoadingUI()` | FinishLoading() 후 UMS::CloseUIByID(LOADING) — PreWarm 완료/스킵 양쪽에서 공용 |

### RSPlayerController — 변경

| 변경 | 내용 |
|------|------|
| `FloatingDamagePool TArray` 멤버 제거 | PoolingSubsystem 위임 |
| `SpawnFloatingDamage()` | `PoolingSubsystem->SpawnPooledWidget()` 호출 |
| `ReturnFloatingDamageToPool()` | `PoolingSubsystem->ReturnWidgetToPool()` 호출 |

---

## 3. 함수 호출 흐름

### 흐름 A — 스테이지 진입 (프리웜 정상)

```
RSGameMode::BeginPlay()
  → InitializePlayer(CharID)
  → InitializeStage()                    // CurrentStageID 설정
  → CachedSpawner 조회
  → InitializePreWarm(Spawner)
      → BuildPreWarmList(Spawner)
          → CollectUniqueEnemyClasses()  // GDS 웨이브 순회
          → MakeActorRequest(EnemyClass, PerClassCount) × N
          → MakeActorRequest(ProjClass, ProjectilePoolCount)
          → MakeWidgetRequest(FloatingDmgClass, WidgetCount)
      → PC->DisableInput()
      → bIsPreWarmActive = true
      → PoolingSubsystem->OnPreWarmComplete.AddUObject(OnPreWarmCompleted)
      → PoolingSubsystem->RequestAsyncPreWarm(PreWarmList)

[매 프레임 Tick]
RSGameMode::Tick()
  → UpdatePreWarmProgress()
      → PoolingSubsystem->GetPreWarmProgress()   // 0.0→1.0
      → GetLoadingWidget()->SetLoadingProgress()

PoolingSubsystem::Tick()
  → TickPreWarm()
      → SpawnOnePreWarmUnit(Req)                 // Actor or Widget
          → AddActorToPool(ActorClass)           // Actor 분기
              → TrySpawnActor()
              → ActorPool.FindOrAdd().Add(Actor)
          or
          → CreateWidget() + WidgetPool.Add()    // Widget 분기
      → PreWarmDoneCount++
  → 완료 감지 → OnPreWarmComplete.Broadcast()

RSGameMode::OnPreWarmCompleted()
  → bIsPreWarmActive = false
  → PC->EnableInput()
  → CloseLoadingUI()
      → GetLoadingWidget()->FinishLoading()  // Progress 100% 표시
      → UMS->CloseUIByID(EUIID::LOADING)    // 위젯 닫기
  → StartStageFlow()
      → StageManagerSubsystem->StartStage(CurrentStageID)
```

---

### 흐름 B — 프리웜 목록 비어있을 때 (즉시 시작)

```
InitializePreWarm(Spawner)
  → BuildPreWarmList() → []  (빈 배열)
  → KHS_WARN("PreWarmList 비어있음")
  → CloseLoadingUI()          // LoadingWidget 닫기 (흐름 A와 동일 경로)
  → StartStageFlow() 즉시 호출
```

---

### 흐름 C — 전투 중 투사체 발사 (Actor 풀 사용)

```
RangedEnemy::Fire()
  → PoolingSubsystem->SpawnPooledActor(EnemyProjectileClass, Transform)
      → PopFirstValid(ActorPool[EnemyProjectileClass])  // 풀에서 꺼냄
          └─ (풀 비어있으면) TrySpawnActor() 신규 스폰
      → Actor->SetActorLocationAndRotation()
      → IPoolableInterface::OnPoolActivate()            // 활성화
      → ActiveActors.Add(Actor)
      → return Actor

EnemyProjectile::OnHit() or Lifetime 만료
  → PoolingSubsystem->ReturnToPool(this)
      → ActiveActors.Remove()
      → IPoolableInterface::OnPoolDeactivate()          // 비활성화
      → ActorPool[class].Add(Actor)
```

---

### 흐름 D — 데미지 플로팅 위젯 사용 (Widget 풀 사용)

```
RSPlayerController::SpawnFloatingDamage(Damage, ScreenPos)
  → PoolingSubsystem->SpawnPooledWidget(FloatingDamageClass, PC)
      → PopFirstValid(WidgetPool[FloatingDamageClass])  // 풀에서 꺼냄
          └─ (풀 비어있으면) CreateWidget() 신규 생성
      → ActiveWidgets.Add(Widget)
      → return Widget
  → Widget->SetPositionInViewport(ScreenPos)
  → Widget->AddToViewport(ZOrder_PAGE)  // 미등록 시만
  → Widget->PlayFloatAnimation(Damage)

[애니메이션 완료 후]
RSPlayerController::ReturnFloatingDamageToPool(Widget)
  → PoolingSubsystem->ReturnWidgetToPool(Widget)
      → ActiveWidgets.Remove()
      → Widget->SetVisibility(Collapsed)
      → WidgetPool[class].Add(Widget)
```

---

### 흐름 E — 무기 교체 (기존 풀 정리 + 새 풀 초기화)

```
EquipmentSubsystem::EquipWeapon(WeaponID)
  → CommitSlot()
      → ClearWeaponPool(OldWeaponID)           // 기존 무기 풀 제거
          → PoolingSubsystem->DrainPool(OldProjClass)
              → 활성 투사체 강제 반납
              → ActorPool.Remove(OldProjClass)
      → InitWeaponPool(NewWeaponID)            // 새 무기 풀 초기화
          → GDS->GetSkillExecutionData()
          → PoolingSubsystem->InitializePool(ProjClass, 10)
          → PoolingSubsystem->InitializePool(SummonClass, 5)
```

---

### 흐름 F — Widget PreWarm 중 PC 미준비 상황

```
PoolingSubsystem::TickPreWarm()
  → Back = PreWarmQueue.Last()  // WidgetClass 요청
  → SpawnOnePreWarmUnit(Back)
      → GetWorld()->GetFirstPlayerController()  // null 반환
      → return false  // PC 미준비 신호
  → break  // 이번 프레임 중단, Count 감소 없음
  → [다음 프레임 재시도]
```

---

## 4. 구조체 FPoolPreWarmRequest

```cpp
USTRUCT(BlueprintType)
struct FPoolPreWarmRequest
{
    TSubclassOf<AActor>      ActorClass  = nullptr;  // Actor 요청
    TSubclassOf<UUserWidget> WidgetClass = nullptr;  // Widget 요청
    int32                    Count       = 0;
    // ActorClass / WidgetClass 중 하나만 유효하면 됨
};
```

---

## 5. ZOrder 상수 (UIManagerSubsystem)

```cpp
static constexpr int32 ZOrder_PERSISTENT = 100;
static constexpr int32 ZOrder_PAGE       = 200;  // FloatingDamage 사용
static constexpr int32 ZOrder_POPUP_BASE = 300;
static constexpr int32 ZOrder_POPUP_STEP =  10;
static constexpr int32 ZOrder_SYSTEM     = 500;
```
