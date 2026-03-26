# PLAN_StageSpawner_v1.0

> 작성일: 2026-03-26
> 관련 기획서: 스테이지_스폰 시스템 기획 v1.2.md, AI_에너미 시스템 기획 v1.1.md
> 스프린트: SPRINT 1 — Task B

---

## 구현 목표

`UStageManagerSubsystem`(WorldSubsystem)과 `AEnemySpawner`(맵 배치 Actor)를 신규 구현하여, **GDS → DT_Stage / DT_WaveData 기반** 웨이브 파라미터로 에너미를 주기적으로 스폰하는 스테이지 스폰 파이프라인을 완성한다.

- GDS의 기존 `GetStageData()` / `GetWaveDataByStage()` API를 그대로 활용.
- 웨이브 전환(StartTime 기반 타이머)까지 이번 스프린트에 포함.
- 기존 `ARSGameMode::InitializePools()` 호출을 `AEnemySpawner`로 이관.
- `AEnemyBaseCharacter`에 `IPoolableInterface` 구현 완성.
- `AEnemyAIController`에 초기 목표 위치 주입 인터페이스 추가.

---

## 영향 범위

### 수정/생성 C++ 클래스

| 구분 | 클래스 | 파일 위치 |
|------|--------|-----------|
| 신규 | `UStageManagerSubsystem` | Public/Subsystems/StageManagerSubsystem.h/.cpp |
| 신규 | `AEnemySpawner` | Public/System/EnemySpawner.h/.cpp |
| 수정 | `ARSGameMode` | Core/RSGameMode.h/.cpp |
| 수정 | `AEnemyBaseCharacter` | Character/Enemy/EnemyBaseCharacter.h/.cpp |
| 수정 | `AEnemyAIController` | Character/Enemy/EnemyAIController.h/.cpp |

### DataTable

없음 (구조체/테이블/GDS API 이미 존재). **에디터에서 DT_Stage + DT_WaveData 행 입력 필요** (아래 §에디터 작업 참조).

---

## 클래스 설계

### `UStageManagerSubsystem` (WorldSubsystem)

```cpp
// --- Public API ---
void SetSpawner(AEnemySpawner* Spawner);
void StartStage(FName StageID);              // GameMode에서 호출
void RegisterAliveEnemy(AEnemyBaseCharacter* Enemy);
void UnregisterAliveEnemy(AEnemyBaseCharacter* Enemy);
void OnEnemyKilled(FName EnemyID);

// --- Private ---
void OnSpawnTimer();                          // 현재 웨이브 SpawnInterval 타이머 콜백
void ActivateWave(int32 WaveIdx);             // 웨이브 전환 — 스폰 타이머 재설정
FName SelectEnemyIDByWeight() const;          // 현재 웨이브 SpawnEnemyIDs+Weights 사용

// --- Runtime State ---
UPROPERTY()
TArray<TObjectPtr<AEnemyBaseCharacter>> AliveEnemies;  // UPROPERTY로 GC 추적
int32 KillCount = 0;
int32 CurrentWaveIndex = 0;
FTimerHandle SpawnTimerHandle;

// DT에서 로드된 스테이지/웨이브 데이터
FStageStaticData CachedStageData;
TArray<FWaveStaticData> CachedWaveData;       // WaveIndex 오름차순 정렬됨

TWeakObjectPtr<AEnemySpawner> Spawner;        // Actor 수명 분리 — WeakPtr
```

**`StartStage` 흐름**:
1. GDS→`GetStageData(StageID, CachedStageData)` 실패 시 오류 로그 + 중단
2. GDS→`GetWaveDataByStage(StageID)` → `CachedWaveData` 캐싱 (비어있으면 오류 로그 + 중단)
3. 각 웨이브의 `StartTime` 기준으로 개별 타이머 예약
   - `StartTime == 0`인 Wave 0은 즉시 `ActivateWave(0)` 호출
   - 이후 웨이브는 `SetTimer(WaveN_TimerHandle, WaveN.StartTime, false)` 예약
4. `ActivateWave(0)` → `SetTimer(SpawnTimerHandle, Wave0.SpawnInterval, bLoop=true)`

**`SelectEnemyIDByWeight`**:
```
CachedWaveData[CurrentWaveIndex].SpawnEnemyIDs / SpawnWeights 사용
두 배열 길이 불일치 시 경고 로그 + 첫 번째 ID 반환
```

### `AEnemySpawner`

```cpp
// --- Public API ---
void InitPools();                              // GameMode가 명시 호출 — PoolInit 이관
void SpawnEnemy(FName EnemyID, const FVector& PlayerLocation);

// --- BP 설정 ---
UPROPERTY(EditDefaultsOnly, Category = "Spawn")
TMap<FName, TSubclassOf<AEnemyBaseCharacter>> EnemyIDToClassMap;

UPROPERTY(EditDefaultsOnly, Category = "Spawn")
float OffScreenDistance = 1500.f;

UPROPERTY(EditDefaultsOnly, Category = "Spawn")
int32 PoolCountPerClass = 30;

// --- Private ---
FVector CalculateOffScreenSpawnLocation(const FVector& PlayerLocation) const;
```

> `EnemyIDToClassMap`의 키 집합이 DT에 입력할 EnemyID와 일치해야 함. BP 작업 시 주의.

### `AEnemyBaseCharacter` — IPoolableInterface 추가

```cpp
virtual void OnPoolActivate() override;
virtual void OnPoolDeactivate() override;
```

**OnPoolActivate**:
1. `SetActorHiddenInGame(false)` + Collision 활성
2. `ASC->InitAbilityActorInfo(this, this)` 재호출
3. DefaultAttributes GE 재적용 → HP 리셋
4. `bIsInitialized = false` (재초기화 허용)

**OnPoolDeactivate**:
1. `ASC->CancelAllAbilities()` (진행 중 Ability 강제 종료)
2. `SetActorHiddenInGame(true)` + Collision 비활성
3. AI 중단: `Cast<AEnemyAIController>(GetController())->StopAI()`

**HandleDeath 수정**:
```
1. bIsDead 중복 가드
2. OnEnemyKilledDel.Broadcast(EnemyID)
   └─ StageManager->OnEnemyKilled(EnemyID): KillCount++
3. StageManager->UnregisterAliveEnemy(this)  ← ReturnToPool 전에 먼저 제거
4. 딜레이 후 PoolingSubsystem->ReturnToPool(this)
   └─ OnPoolDeactivate() 호출
```

### `AEnemyAIController`

```cpp
void SetInitialTargetLocation(const FVector& Location);
// Blackboard::SetValueAsVector(BBKey_PlayerLocation, Location) 즉시 설정
// Tick의 UpdatePlayerInfo 대기 없이 BT 첫 틱부터 유효한 목표 제공
```

### `ARSGameMode` — 역할 축소

```cpp
// BeginPlay
// 1. FPoolInitEntry / InitializePools() 제거
// 2. Spawner = FindActorOfClass<AEnemySpawner>()
// 3. Spawner->InitPools()                       ← PoolInit 이관
// 4. StageManager->SetSpawner(Spawner)
// 5. StageManager->StartStage("Stage_001")       ← 현재는 StageID 하드코딩
```

> `StartStage`의 StageID 인자가 유일하게 하드코딩되는 값. DT_Stage에 행이 있으면 바로 동작.

---

## 함수 호출 흐름

### 스테이지 시작

```
ARSGameMode::BeginPlay()
  ├─ Spawner = FindActorOfClass<AEnemySpawner>()  → null이면 오류 + 중단
  ├─ Spawner->InitPools()
  │    └─ EnemyIDToClassMap 순회 → PoolingSubsystem::InitializePool(Class, Count)
  ├─ StageManager->SetSpawner(Spawner)
  └─ StageManager->StartStage("Stage_001")
       ├─ GDS::GetStageData("Stage_001", CachedStageData)
       ├─ GDS::GetWaveDataByStage("Stage_001") → CachedWaveData (WaveIndex 오름차순)
       ├─ Wave 0 (StartTime=0): ActivateWave(0)
       │    └─ SetTimer(SpawnTimerHandle, Wave0.SpawnInterval, bLoop=true)
       └─ Wave 1,2,3: SetTimer(WaveN_TimerHandle, WaveN.StartTime, bLoop=false)
            └─ 콜백: ActivateWave(N) → 스폰 타이머 재설정(새 SpawnInterval)
```

### 에너미 스폰

```
UStageManagerSubsystem::OnSpawnTimer()
  ├─ AliveEnemies.Num() >= CachedWaveData[CurrentWaveIndex].MaxAliveCount → 스킵
  ├─ PlayerPawn null 체크 → null이면 스킵
  ├─ EnemyID = SelectEnemyIDByWeight()   // 현재 웨이브 SpawnEnemyIDs+Weights 기반
  ├─ Spawner.IsValid() 체크 → invalid이면 경고 + 스킵
  └─ Spawner->SpawnEnemy(EnemyID, PlayerLocation)
       ├─ Class = EnemyIDToClassMap[EnemyID] → null이면 경고 + 스킵
       ├─ SpawnPos = CalculateOffScreenSpawnLocation(PlayerLocation)
       ├─ Enemy = PoolingSubsystem->SpawnPooledActor(Class, Transform)
       │    → null이면 경고 + 스킵 / 내부서 OnPoolActivate() 호출
       ├─ Enemy->InitializeEnemy(EnemyID)
       │    └─ GDS 조회 → AS 초기값 → StartEnemyAI(BT)
       ├─ AEnemyAIController::SetInitialTargetLocation(PlayerLocation)
       └─ StageManager->RegisterAliveEnemy(Enemy)
```

### 웨이브 전환

```
ActivateWave(N)
  ├─ CurrentWaveIndex = N
  ├─ GetWorldTimerManager().ClearTimer(SpawnTimerHandle)  // 이전 타이머 정리
  └─ SetTimer(SpawnTimerHandle, CachedWaveData[N].SpawnInterval, bLoop=true)
```

### 에너미 사망

```
AEnemyBaseCharacter::HandleDeath()
  ├─ bIsDead guard
  ├─ OnEnemyKilledDel.Broadcast(EnemyID) → StageManager->OnEnemyKilled: KillCount++
  ├─ StageManager->UnregisterAliveEnemy(this)
  └─ 딜레이 후 ReturnToPool(this) → OnPoolDeactivate()
```

---

## 에디터 작업 (코드 완료 후)

1. **DT_Stage** 신규 행 입력: `StageID="Stage_001"`, `TimeLimit`, `SpawnEnemyIDs`
2. **DT_WaveData** 신규 행 입력: Wave 0~3 각각 `StageID="Stage_001"`, `WaveIndex`, `StartTime`, `SpawnInterval`, `MaxAliveCount`, `SpawnEnemyIDs`, `SpawnWeights`
3. **BP_EnemySpawner** (AEnemySpawner 기반): `EnemyIDToClassMap` 키를 DT_WaveData에 입력한 EnemyID와 동일하게 매핑

---

## 예외처리 목록

| 상황 | 처리 |
|------|------|
| GDS GetStageData 실패 | 오류 로그 + StartStage 중단 |
| GetWaveDataByStage 빈 배열 | 오류 로그 + StartStage 중단 |
| EnemyIDToClassMap에 ID 없음 | 경고 로그 + 스폰 스킵 |
| PlayerPawn null | 타이머 스킵, 다음 인터벌 재시도 |
| SpawnPooledActor nullptr | 경고 로그 + 스킵 (풀 고갈) |
| TWeakObjectPtr\<Spawner\> invalid | 경고 로그 + OnSpawnTimer 스킵 |
| SpawnEnemyIDs / SpawnWeights 길이 불일치 | 경고 로그 + 첫 번째 ID 반환 |
| FindActorOfClass 실패 | 오류 로그 + StartStage 미호출 |
| HandleDeath 중복 호출 | bIsDead 플래그 가드 |

---

## [검토 결과]

### 기획서 일관성
- 스폰 요청·처치 집계·웨이브 관리 역할 분리가 기획서와 일치.
- 레벨업 연동·세이브 연동·HUD 업데이트·보스 트리거는 이번 스프린트 범위 밖.

### Gemini 리뷰 반영 사항
| 지적 | 반영 여부 | 내용 |
|------|-----------|------|
| StartStage를 GameMode에서 호출 | ✅ | Spawner::BeginPlay에서 직접 호출 제거 |
| TObjectPtr → TWeakObjectPtr | ✅ | Spawner 참조를 WeakPtr로 교체 |
| OnPoolDeactivate 후 AliveEnemies 잔류 | ✅ | HandleDeath에서 ReturnToPool 전 Unregister |
| HP 리셋 위치 (Deactivate→Activate) | ✅ | OnPoolActivate 시 DefaultAttributes GE 재적용 |
| CancelAllAbilities 누락 | ✅ | OnPoolDeactivate에 추가 |
| InitAbilityActorInfo 재호출 | ✅ | OnPoolActivate에 명시 |
| 병렬 배열 OOB 방어 | ✅ | 길이 불일치 시 경고 + fallback |
| Pool 소진 시 흐름 | ✅ | null 반환 즉시 스킵 처리 |

### 기획서 정정 필요 사항
| # | 대상 문서 | 섹션 | 정정 내용 |
|---|----------|------|-----------|
| 1 | 스테이지_스폰 기획 v1.2 | 스포너↔스테이지 관계표 | Spawner가 Subsystem에 자신을 등록하는 흐름 보완 필요 |
| 2 | 스테이지_스폰 기획 v1.2 | 데이터 설계 | `FStageStaticData` 현재 구현체에 `BossTriggerTime`, `BossEnemyID`, `RewardGold` 등 기획서 컬럼 미구현 상태. 다음 스프린트 추가 예정임을 기획서에 명시 필요 |

### 기획서 미정의 → 이번 스프린트 자체 결정
- StageManagerSubsystem: WorldSubsystem (PoolingSubsystem과 동일 패턴)
- EnemySpawner 오프스크린 위치: 단순 고정 반경 오프셋 (NavMesh는 다음 스프린트 TODO)
- Pool 초기화 호출 위치: GameMode::BeginPlay에서 Spawner->InitPools() 명시 호출
