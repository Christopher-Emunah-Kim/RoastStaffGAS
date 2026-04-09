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

### [2026-04-09] [ARCH] StageSelectWidget 복원 — 저장 시점 선택과 기존 함수 재사용

**상황**: 로비 복귀 시 스테이지 선택 화면이 항상 초기화 상태로 시작 — 재도전 UX 단절.

**문제·과제**: LastPlayedStageID를 언제 저장할지, 복원 시 UpdateDetailPanel / SelectedStageID / Btn_Confirm 3개 상태를 중복 없이 정확히 복원하는 방법.

**검토한 선택지**:
- 저장 시점: OnStageSelected(진입 확정) vs UpdateStageRecord(스테이지 종료) — UpdateStageRecord는 실패 케이스도 포함하므로 "마지막 진입 스테이지" 의미와 불일치
- 복원 방법: OnNodeClicked 재사용 vs 3개 상태 직접 조작 — 직접 조작 시 OnNodeClicked 내부 변경에 취약 (LOCKED 방어 로직 누락 위험)

**결정**: 저장은 OnStageSelected에서 SaveGame() 직전 일괄 (세이브 기획서 트리거 정책 준수). 복원은 OnNodeClicked 재사용 — LOCKED 방어 자동 처리 + DRY 보장. PopulateNodeMap() 이후 순서 고정으로 NodeDataCache 선행 조건 보장.

**결과**: 6개 파일 수정, 신규 파일 없음. 재도전 UX 복원 완료.

**포트폴리오 포인트**: 기존 함수 재사용으로 3개 상태 동기화 문제를 없앤 판단 + 저장 시점의 의미 정합 고려 (진입 확정 vs 종료 결과).

**관련 파일**: RSGameSave.h, SaveGameSubsystem.h/.cpp, RSOutGamePlayerController.cpp, RSStageSelectWidget.h/.cpp

---

### [2026-04-08] [ARCH] Private 헬퍼 배치 — anonymous namespace vs private member

**상황**: PoolingSubsystem과 RSGameMode 리팩토링 중 `.cpp`에서만 사용되는 헬퍼 함수(`PopFirstValid<T>`, `SpawnOnePreWarmUnit`, `CollectUniqueEnemyClasses` 등)의 배치 위치를 결정해야 했다.

**문제/과제**: `anonymous namespace`에 두면 헤더 노출 없이 완전 은닉 가능하나, UE 리플렉션/UFUNCTION 매크로와 함께 사용 불가. `private member`로 두면 헤더에 선언이 필요하다.

**검토한 선택지**:
  - A) `anonymous namespace` — 완전 은닉, 외부 노출 없음. 그러나 UFUNCTION 불가, this 포인터 접근 시 매개변수 전달 필요
  - B) `private member` — 헤더 노출, 클래스 인터페이스 오염. 단 UFUNCTION 가능, this 자연 참조

**결정**: B 선택. UE 코드베이스에서 헬퍼 함수는 `private member`가 표준이며, `anonymous namespace`는 순수 static 유틸리티(UObject 무관)에만 허용. conventions.md에 원칙 추가.

**결과/효과**: 모든 private 헬퍼에 헤더 선언 확보. 팀 규칙으로 정착.

**포트폴리오 포인트**: UE 코드 구조 관례 이해 / C++ 은닉 메커니즘 선택 근거 설명 능력

**관련 파일**: `Source/RoastStaffGAS/Public/Subsystems/PoolingSubsystem.h`, `.claude/skills/coding/references/conventions.md`

---

### [2026-04-08] [REFACTOR] PopFirstValid<T> 템플릿 — Actor/Widget Pool Pop 루프 중복 제거

**상황**: `SpawnPooledActor`, `SpawnPooledWidget` 각각의 Pop 루프가 동일한 구조(IsValid 체크 → 유효하지 않은 객체 건너뛰기)를 중복 구현하고 있었다.

**문제/과제**: 같은 로직이 Actor/Widget 두 경로에 복사되어 있어, Pop 정책 변경 시 2곳을 동기화해야 함.

**검토한 선택지**:
  - A) 현행 유지 — 구현 단순, Pop 정책 변경 시 두 곳 수정 필요
  - B) `PopFirstValid<T>(TArray<T*>&)` 템플릿 함수 추출 — 단일 정의, Actor/Widget 모두 재사용

**결정**: B 선택. IsValid가 UObject 계층에서 공통으로 동작하므로 템플릿화 가능. 함수 시그니처: `template<typename T> T* PopFirstValid(TArray<T*>& Pool)`.

**결과/효과**: Pop 루프 중복 제거. Pop 정책 변경 시 단일 지점만 수정.

**포트폴리오 포인트**: UE에서 UObject 계층 공통성을 활용한 템플릿 설계 / 중복 제거와 YAGNI 균형

**관련 파일**: `Source/RoastStaffGAS/Public/Subsystems/PoolingSubsystem.h`, `Private/Subsystems/PoolingSubsystem.cpp`

---

### [2026-04-08] [BUG_FIX] constexpr 타입 절삭 — int32에 float 리터럴 대입

**상황**: SR-FULL(2026-04-08)에서 발견. `GA_ProjectileAttack.cpp`의 발사 각도 클램프 상수 `constexpr int32 LAUNCH_ANGLE_CLAMP = 80.f`.

**문제/과제**: `int32` 타입에 `80.f` float 리터럴 대입 → 묵시적 절삭(truncation) 발생. 현재는 80이라 값 손실 없지만 컴파일러 경고(C4244) 대상이고, 향후 소수점 값으로 바뀔 경우 실제 손실.

**검토한 선택지**:
  - A) `constexpr float LAUNCH_ANGLE_CLAMP = 80.f` — 사용처(`FMath::Clamp<float>`)와 타입 일치
  - B) `constexpr int32 LAUNCH_ANGLE_CLAMP = 80` — 정수 리터럴로 통일

**결정**: A 선택. 클램프 대상이 float 연산이므로 float 타입이 자연스럽다.

**결과/효과**: 타입 불일치로 인한 암묵적 변환 제거. "constexpr 선언 시 사용처 타입에 맞춰 선언" 규칙으로 내면화.

**포트폴리오 포인트**: C++ 타입 시스템과 컴파일러 경고 주의 / constexpr 선언 시 타입 정합성 점검 습관

**관련 파일**: `Source/RoastStaffGAS/Private/GAS/Abilities/GA_ProjectileAttack.cpp`

---

### [2026-04-08] [ARCH] Enemy 계층 투사체 발사 중복 — Template Method 추출 계획

**상황**: SR-FULL(2026-04-08)에서 신규 발견. `ARangedEnemy::FireProjectile`, `AEliteEnemy::FireProjectile`, `ABossEnemy::LaunchProjectileInDirection`이 동일한 3단계(풀 획득 → InitEnemyProjectile → 방향 계산)를 반복 구현. 클래스별 float 파라미터(`PreferredRange`, `MaxAttackRange`, `ProjectileSpeed`, `ProjectileLifetime`, `AttackDamage`)도 3개 클래스에 15개 중복 선언.

**문제/과제**: 투사체 발사 정책 변경 시 3곳 동시 수정 필요. 파라미터 필드도 15개가 UPROPERTY 없는 plain float로 중복.

**검토한 선택지**:
  - A) 현행 유지 — 클래스 독립성 높으나 변경 비용 3배
  - B) `EnemyBaseCharacter`에 `LaunchEnemyProjectile(Direction, Damage, GEClass)` 공통 헬퍼 추출 + 파라미터를 `FEnemyRangedParams` USTRUCT로 통합

**결정**: B 예정 (다음 스프린트). 공통 헬퍼는 Template Method 패턴, 파라미터 구조체는 DataTable 외부화와 자연스럽게 연결.

**결과/효과**: (구현 예정) 발사 정책 단일 지점 수정. Enemy 종류 추가 시 파라미터 재선언 불필요.

**포트폴리오 포인트**: 계층 구조에서 반복 패턴 식별 능력 / Template Method + USTRUCT 조합 설계

**관련 파일**: `Source/RoastStaffGAS/Public/Character/Enemy/EnemyBaseCharacter.h` (예정), `RangedEnemy.h`, `EliteEnemy.h`, `BossEnemy.h`

---

### [2026-04-07] [PATTERN] Instigator 패턴 — 투사체 자기 충돌 방지

**상황**: 에너미가 발사한 투사체가 발사 직후 자기 자신과 충돌하는 버그. 초기 접근은 발사 오프셋 하드코딩(`SPAWN_OFFSET = 200.f`)이었다.

**문제/과제**: 오프셋 방식은 캡슐 크기·충돌 채널 변경에 취약하고, 오프셋 값 자체가 또 다른 하드코딩.

**검토한 선택지**:
  - A) 발사 오프셋 하드코딩 — 즉각 적용 가능, 물리 레이어 변경에 취약
  - B) Instigator 패턴 — `Projectile->SetInstigator(OwnerEnemy)` + `SphereComp->IgnoreActorWhenMoving(GetInstigator())` 조합

**결정**: B 선택. 의미 기반 무시 — "발사자를 무시한다"는 의도가 코드에 직접 표현됨. 물리 설정 변경에 독립적.

**결과/효과**: 오프셋 없이 자기 충돌 방지. Elite/Boss 투사체에도 동일 패턴 재사용.

**포트폴리오 포인트**: 하드코딩 vs 의미 기반 솔루션 선택 / UE Instigator API 실전 활용

**관련 파일**: `Source/RoastStaffGAS/Private/Character/Enemy/EnemyProjectile.cpp`

---

### [2026-04-07] [PATTERN] UE_LOG Verbose 필터링 — 진단 로그가 안 보이는 함정

**상황**: 디버깅 중 `KHS_DEBUG` 카테고리로 `Verbose` 레벨 로그를 출력했는데, Output Log에 아무것도 표시되지 않아 "버그 없음"으로 오판할 뻔했다.

**문제/과제**: UE5 Output Log의 기본 필터는 `Display` 이상이라 `Verbose`, `VeryVerbose`는 기본 상태에서 표시되지 않는다. 에러나 경고 없이 조용히 숨겨진다.

**근본 원인**: `UE_LOG(KHS_DEBUG, Verbose, ...)` → Output Log 기본 필터에서 숨김. 에디터 필터를 명시적으로 낮추거나 카테고리 로그 레벨을 올려야 표시됨.

**결정**: 진단 중 로그는 `KHS_INFO`(`Display` 레벨) 이상 사용. `Verbose`는 배포 시 억제 목적이므로 진단 단계에서 부적합.

**결과/효과**: "로그 없음 = 정상 동작" 오독 함정 제거. UE_LOG 6단계(Fatal/Error/Warning/Display/Verbose/VeryVerbose)와 Output Log 필터 기준 내면화.

**포트폴리오 포인트**: UE 로깅 시스템 깊이 이해 / 진단 도구를 올바르게 사용하는 습관

**관련 파일**: `Source/RoastStaffGAS/Public/Core/Logging/RSLoggingSystem.h`

---

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

## 2026-03

### [2026-03-31] [PATTERN] UE5 빌드 셋업 함정 3가지

**상황**: 새 모듈과 UENUM을 추가하는 과정에서 컴파일 오류와 git 파싱 오류가 연달아 발생. 각각의 에러 메시지가 직접적이지 않아 진단에 시간이 걸렸다.

**함정 1 — UENUM과 .generated.h**:
UENUM()이 있는 헤더에는 반드시 `#include "파일명.generated.h"` 가 있어야 한다. 없으면 UHT 오류. 일반 `enum class`와 달리 UENUM은 UHT 코드 생성을 전제한다.

**함정 2 — UDeveloperSettings와 Build.cs**:
`UDeveloperSettings`를 상속하는 클래스 작성 시 `Build.cs`의 `PublicDependencyModuleNames`에 `"DeveloperSettings"` 모듈을 명시적으로 추가해야 한다. 누락 시 링크 오류.

**함정 3 — 한글 파일명과 UBT 크래시**:
프로젝트 경로나 애셋 이름에 한글이 포함된 경우, `git config core.quotepath false` 설정이 없으면 UBT가 git 경로를 octal 이스케이프 문자열로 파싱해 .NET 크래시 발생.

**결과/효과**: 세 가지 모두 팀 온보딩 체크리스트 수준의 비자명한 함정. 이후 새 팀원 환경 셋업 시 선제적으로 안내.

**포트폴리오 포인트**: UE 빌드 시스템 심층 이해 / 환경 셋업 트러블슈팅 경험

**관련 파일**: `RoastStaffGAS.Build.cs`, `Config/DefaultEngine.ini`

---

### [2026-03-30] [ARCH] EquipWeapon 강화 설계 — IncomingData(트리거) vs SlotData(실행) 역할 분리

**상황**: SR(2026-03-30)에서 `EquipmentSubsystem::EquipWeapon` 강화 판정 로직 리뷰 중 발견. `GetWeaponData(WeaponID, IncomingData)` 조회 후 `BaseType` 비교에만 사용하고, `NextLevelWeaponID`는 SlotData 기준으로만 처리.

**문제/과제**: 리뷰어 관점에서 들어오는 카드(IncomingData)의 `NextLevelWeaponID`를 쓰지 않는 게 의도인지 버그인지 불분명했다.

**검토한 선택지**:
  - A) 들어오는 카드(IncomingData) 기준 강화 — 새 카드의 NextLevel로 올림
  - B) 현재 슬롯(SlotData) 기준 강화 — 이미 장착된 무기의 NextLevel로 올림

**결정**: B가 올바른 설계. 기획 의도: 강화는 "현재 슬롯 무기의 레벨을 올리는 것". 들어오는 카드는 "같은 종류(BaseType) 트리거" 역할만 한다. IncomingData의 BaseType 비교 전용 사용이 정확한 설계.

**결과/효과**: 의도적 설계 확인(RESOLVED). 다만 코드에 의도 주석이 없어 리뷰어 혼란이 발생 — 변수명·주석으로 의도 명시 개선 검토.

**포트폴리오 포인트**: 기획 의도가 코드에 올바르게 반영됐는지 검증하는 리뷰 프로세스 / "트리거 역할 vs 데이터 역할" 인터페이스 설계 사고

**관련 파일**: `Source/RoastStaffGAS/Private/Subsystems/EquipmentSubsystem.cpp` (~88~105라인, EquipWeapon)

---

### [2026-03-27] [BUG_FIX] SetInputMode + SetShowMouseCursor 쌍 처리 — 수동 발사 클릭 10회 중 2회만 인식

**상황**: 수동 발사 모드에서 클릭 10회 중 2회만 `RequestManualFire`에 도달하는 버그.

**문제/과제**: 처음에는 GAS 내부 문제(`bRetriggerInstancedAbility`)로 오판, MODULE-2 코드를 수정했다가 롤백. 실제 원인은 입력 레이어였다.

**근본 원인**: 팝업 닫힐 때 `FInputModeGameOnly()` 복원 후 `SetShowMouseCursor(true)` 호출 누락. Slate가 마우스 관리권을 유지한 채 클릭을 게임에 전달하지 않음. `SetShowMouseCursor(false)` 제거 시 같은 스코프의 `SetShowMouseCursor(true)`(팝업 없음 분기)까지 함께 삭제됨 — "한 줄 삭제"가 두 분기 모두에 영향.

**검토한 선택지**:
  - A) GAS `bRetriggerInstancedAbility` 수정 — 실제 문제 레이어가 아님 (롤백)
  - B) 입력 레이어 진단: `RequestManualFire` 진입 여부 먼저 확인 → `SetInputMode` + `SetShowMouseCursor` 쌍 복원

**결정**: B. `git diff`로 최근 변경 코드를 먼저 확인했다면 첫 단계에서 원인 특정이 가능했다.

**결과/효과**: 규칙 확립: `SetInputMode` + `SetShowMouseCursor` + `SetConsumeCaptureMouseDown`은 항상 쌍으로 검토. 진단 순서 원칙: "진입점 확인 → 레이어 좁히기 → 내부 진단".

**포트폴리오 포인트**: UE5 Slate 입력 캡처 메커니즘 이해 / 잘못된 레이어 진단 → 롤백 → 올바른 순서 복구 과정 (사고 과정 투명성)

**관련 파일**: `Source/RoastStaffGAS/Private/Character/Player/RSPlayerController.cpp`

---

## [2026-04-09] ARCH — 보스 HP Bar 위젯 이중 Close 타이밍 충돌 처리

**상황**: 보스 HP Bar(WBP_BossHPBar)는 HP=0 감지 시 FadeOut 애니메이션을 재생하고, 완료 후 UIManagerSubsystem에 정리를 요청하는 구조. 그러나 보스 사망 이벤트(OnBossKilled)가 FadeOut 도중 EnemySpawner에 먼저 도달해 `CloseUIByID`를 즉시 호출하면 FadeOut이 강제 중단되고, UMS의 `CloseUIInternal` 이 `RemoveFromParent`까지 즉시 실행해 애니메이션이 끊기는 문제가 존재.

**문제·과제**: 위젯이 FadeOut을 자율적으로 처리하면서도 UMS의 PERSISTENT 레이어 정리(PersistentUIMap cleanup)까지 보장해야 함. `CloseUI()` 오버라이드만으로는 UMS가 `CloseUI` 직후 `RemoveFromParent`를 호출하는 구조를 막을 수 없음.

**검토한 선택지**:
- `CloseUI()` 오버라이드: FadeOut 재생 후 `Super::CloseUI()` 반환 → UMS가 바로 `RemoveFromParent` 호출해 효과 없음
- `mutable bIsClosing` + EnemySpawner 게이트: EnemySpawner가 `IsClosing()` 확인 후 `CloseUIByID` 스킵 → FadeOut 완료 후 위젯이 UMS에 직접 `CloseUIByID` 요청, UMS가 정상 정리 수행

**결정**: `bIsClosing` 플래그를 위젯 내부에 두고, EnemySpawner가 `TWeakObjectPtr<UBossHPBarWidget>` 캐시로 상태를 조회. FadeOut 완료 시 위젯이 `GetWorld()->GetGameInstance()`를 통해 UMS `CloseUIByID`를 직접 호출해 PersistentUIMap 정리까지 보장.

**결과**: FadeOut 중 `OnBossKilled` 도달 시 EnemySpawner 게이트에서 `CloseUIByID` 스킵. FadeOut 없는 경우(Anim_FadeOut 미설정)는 EnemySpawner 폴백 경로로 즉시 정리. 두 경로 모두 UMS 정리 완결.

**포트폴리오 포인트**: 위젯-UMS 간 비동기 생명주기 충돌을 플래그+캐시 조합으로 조율. UMS `CloseUIInternal`의 `CloseUI → RemoveFromParent` 즉시 호출 구조를 분석하고, 위젯 자율 종료 패턴과 UMS 중앙 관리 원칙을 모두 만족하는 설계를 도출한 사례.

**관련 파일**: `Source/RoastStaffGAS/Public/UI/Enemy/BossHPBarWidget.h`, `Source/RoastStaffGAS/Private/System/EnemySpawner.cpp`

---

<!-- 새 항목은 가장 최근 날짜가 위로 오도록 추가 -->