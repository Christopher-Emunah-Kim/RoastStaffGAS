# DEVLOG — RoastStaffGAS 기술 의사결정 로그
> 포트폴리오·기술문서용. 파이프라인(PLAN/CODE/SR)에서 자동 기록.
> 독자: 면접관, 팀장, 채용 담당자 — 기술 판단력을 보여주는 서사 중심.

## 기록 기준 (아래 중 하나 해당 시만 기록)
- 선택지 2개 이상을 실제로 검토한 설계 결정
- 버그 원인이 즉각 자명하지 않았던 경우 (진단 과정 포함)
- 성능·메모리·구조 트레이드오프가 명확히 존재
- UE/GAS/C++의 비자명한 패턴 활용 (UPROPERTY 생명주기, ASC 소유권 등)

## 타입
| 타입 | 의미 |
|------|------|
| `ARCH` | 구조/설계 결정 — 클래스 책임 분리, 시스템 경계 |
| `BUG_FIX` | 비자명한 버그 — 원인 진단 과정이 핵심 |
| `OPT` | 성능·메모리 최적화 — 측정 가능한 개선 |
| `REFACTOR` | 구조 개선 — 기능 변화 없이 설계 품질 향상 |
| `PATTERN` | UE/GAS/C++ 특화 패턴 발견 및 적용 |

---

## 항목 형식
```
## [YYYY-MM-DD] [TYPE] 제목

**상황**: 어떤 맥락에서 이 결정이 필요했나
**문제/과제**: 정확히 무엇을 해결해야 했나
**검토한 선택지**:
  - A) ... — 장단점
  - B) ... — 장단점
**결정**: 무엇을 선택했고 왜
**결과/효과**: 실제로 어떻게 됐나
**포트폴리오 포인트**: 이 항목이 보여주는 역량
**관련 파일**: Source/... (줄번호 선택)
```

---

## 2026-04

### [2026-04-08] [ARCH] PoolingSubsystem AsyncPreWarm — UTickableWorldSubsystem 기반 배치 스폰

**상황**: 인트로 로딩 중 Enemy Actor와 Widget을 미리 스폰해 Pool에 적재해야 했다.
이전 구조는 EnemySpawner가 직접 InitializePool을 호출하는 방식으로, 호출 위치가 분산되어 있었다.

**문제/과제**: 스테이지 시작 전 수십 개의 Actor를 한 프레임에 스폰하면 프레임 스파이크 발생.
로딩 UI와 연동하는 완료 이벤트도 필요했다.

**검토한 선택지**:
  - A) BeginPlay 일괄 스폰 — 구현 단순, 그러나 한 프레임에 집중되는 스폰 비용으로 히칭 발생
  - B) Tick 분산 배치 스폰 (UTickableWorldSubsystem 변환) — 프레임당 N개 제한, 외부 튜닝 가능
  - C) AsyncTask 오프로드 — Actor 스폰이 GameThread 의존이라 실질적으로 불가

**결정**: B 선택.
`UTickableWorldSubsystem`으로 변경해 매 Tick에 `PreWarmBatchSize`개씩 스폰.
`GetPreWarmProgress()` float 반환으로 로딩 바 연동, `OnPreWarmComplete` FSimpleMulticastDelegate로 완료 이벤트 노출.

**결과/효과**: 스폰 비용이 여러 프레임으로 분산. 완료 델리게이트를 통해 GameMode가 PreWarm 종료 후 스테이지 진입 로직을 구동.

**포트폴리오 포인트**: UTickableWorldSubsystem 활용한 프레임 예산 분산 패턴 / 델리게이트 기반 비동기 완료 통지 설계

**관련 파일**: `Source/RoastStaffGAS/Public/Subsystems/PoolingSubsystem.h`, `Private/Subsystems/PoolingSubsystem.cpp`

---

### [2026-04-08] [BUG_FIX] ActorPool GC 크래시 — TArray<AActor*> UPROPERTY 누락

**상황**: `PoolingSubsystem`이 스폰한 Actor를 TArray에 보관하고 있었는데,
특정 조건(장시간 플레이, GC 수행 타이밍)에서 풀에서 꺼낸 Actor가 이미 소멸된 상태로 반환됐다.

**문제/과제**: TArray<AActor*>는 UPROPERTY가 없으면 UE GC의 추적 대상에서 제외된다.
GC 사이클에서 "참조 없음"으로 판단해 Actor를 수집(collect)해버림.

**검토한 선택지**:
  - A) TArray에 UPROPERTY 추가 — 가장 단순, 하지만 USTRUCT 내부 TArray는 UPROPERTY 불가
  - B) TArray를 USTRUCT(`FActorPoolBucket`)로 래핑 후 UPROPERTY 선언 — GC 추적 보장
  - C) TWeakObjectPtr 사용 — 약참조라 스폰된 Actor를 GC로부터 보호 불가

**결정**: B 선택. `FActorPoolBucket` USTRUCT를 도입해 `TArray<AActor*> Actors`를 감싸고,
`TMap<TSubclassOf<AActor>, FActorPoolBucket>`에서 UPROPERTY로 선언.

**결과/효과**: GC가 FActorPoolBucket 내부 Actors를 추적하게 되어 크래시 재현 불가.
`FWidgetPoolBucket`도 동일 패턴으로 구현해 Widget 풀에도 적용.

**포트폴리오 포인트**: UE GC 추적 메커니즘 이해 / USTRUCT 래퍼를 통한 UPROPERTY 강참조 확보 패턴

**관련 파일**: `Source/RoastStaffGAS/Public/Subsystems/PoolingSubsystem.h` (`FActorPoolBucket`, `FWidgetPoolBucket`)

---

### [2026-03] [PATTERN] UMG AddDynamic 바인딩 위치 — NativeOnInitialized vs NativeConstruct

**상황**: Widget을 Pool에서 꺼내 재사용할 때 델리게이트가 중복 바인딩되는 버그를 발견했다.

**문제/과제**: 처음에는 GAS AttributeSet이나 이벤트 발송 로직 문제로 오해했다.
진단 순서를 잘못 잡아 내부 로직부터 살펴봤다가 방향을 틀어야 했다.

**근본 원인**: `AddDynamic`을 `NativeConstruct`에 배치하면,
Widget을 Pool에서 꺼낼 때(재활성화 시) `NativeConstruct`가 다시 호출되어 동일 델리게이트에 중복 등록됨.

**결정**: `AddDynamic` 바인딩 전체를 `NativeOnInitialized`로 이전.
`NativeOnInitialized`는 Widget 최초 생성 시 1회만 호출되므로 재사용 시 중복 없음.

**결과/효과**: 풀링 Widget 재사용 시 이벤트 중복 실행 버그 완전 해소.
규칙으로 정착: "AddDynamic은 NativeOnInitialized에만".

**포트폴리오 포인트**: UMG Widget 생명주기 깊은 이해 / 풀링 재사용 패턴에서의 생명주기 충돌 진단

**관련 파일**: `Source/RoastStaffGAS/Private/UI/`

---

<!-- 새 항목은 가장 최근 날짜가 위로 오도록 추가 -->