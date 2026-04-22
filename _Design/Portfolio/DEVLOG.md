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

**UE_Ver**: 5.x
**Knowledge_Risk**: LOW | MEDIUM | HIGH
  LOW    — 학습 데이터 내 안정 API
  MEDIUM — cutoff 근처, 다음 버전 업 시 재검증 권장
  HIGH   — cutoff 이후 API, 반드시 재검증

**상황**: 어떤 맥락에서 이 결정이 필요했나
**문제/과제**: 정확히 무엇을 해결해야 했나
**검토한 선택지**:
  - A) ... — 장단점
  - B) ... — 장단점
**결정**: 무엇을 선택했고 왜
**결과/효과**: 실제로 어떻게 됐나
**포트폴리오 포인트**: 이 항목이 보여주는 역량
**관련 파일**: Source/... (줄번호 선택)
**검증 기준**:
  - [ ] (해당 항목이 실제로 해결됐음을 확인하는 구체적 조건)
```

---

## 2026-04

### [2026-04-22] [ARCH] PreWarm 풀 수량 정보의 분산 보유 — 책임 기반 설계 검증

**UE_Ver**: 5.4
**Knowledge_Risk**: LOW

**상황**: 스킬 이펙트 액터(PullVortexActor 등)를 PreWarm에 추가하면서, 풀 수량이 GameMode/EnemySpawner/PC에 분산되어 있는 구조가 의도된 것인지 설계 결함인지 점검이 필요했다.

**문제/과제**: `BuildPreWarmList`가 EnemySpawner, PlayerController, RuntimeDataSubsystem, GameDataSubsystem 등 여러 소스에서 데이터를 수집하는 구조. "GameMode 단일 집중"이 맞는지, "책임별 분산"이 맞는지 판단.

**검토한 선택지**:
  - A) GameMode 단일 집중 — 모든 풀 수량을 GameMode의 `EditDefaultsOnly`로 관리. 조회 지점이 하나지만, GameMode가 에너미·위젯·스킬 세부사항을 모두 알아야 해 단일 책임 위반.
  - B) 책임 기반 분산 (현재 구조) — EnemySpawner는 스테이지 구성(에너미 종류·수량)을, PC는 자신의 UI 위젯 클래스를, GameMode는 GameMode 수준 수량만 보유. `BuildPreWarmList`는 이를 "조율"하는 역할만 담당.

**결정**: B안이 올바른 설계. `BuildPreWarmList`는 데이터 소유자가 아니라 PreWarm 요청 조립자이며, 오케스트레이터인 GameMode의 책임 범위 안에 있다. 각 소유자는 자신의 도메인 안에서 값을 결정하고, GameMode는 이를 수집해 PoolingSubsystem에 위임한다.

**결과/효과**: 스킬 이펙트 액터 PreWarm 추가 시 기존 패턴을 그대로 따라 자연스럽게 확장됨. 설계 의도를 명확히 인지함으로써 이후 PreWarm 항목 추가 시 올바른 위치(소유자)에 수량을 두는 기준이 생겼다.

**포트폴리오 포인트**: 동작하는 코드에서 의도를 역추적해 설계 원칙(단일 책임, 오케스트레이터 패턴)과 대조·검증하는 사고 과정.

**관련 파일**: Source/RoastStaffGAS/Private/Core/RSGameMode.cpp (BuildPreWarmList), Public/System/EnemySpawner.h

---

### [2026-04-20] [BUG_FIX] AGroundEffectActor 충돌 활성화 타이밍 버그 — OnPoolActivate vs InitGroundEffect

**UE_Ver**: 5.4
**Knowledge_Risk**: LOW

**상황**: 장판(GroundEffectActor)이 스폰되자마자 근처 에너미에게 GE가 적용되는 버그. 로그에 "필수 데이터 누락" 경고가 찍히면서 ApplyGEToTarget이 null 체크에서 return.

**문제/과제**: 충돌 감지는 되는데 GE 적용에 필요한 ASC/GEClass 캐시가 null인 상태. 원인 진단이 필요했다.

**검토한 선택지**:
  - A) `OnPoolActivate`에서 충돌 활성화 (기존 구현) — 풀에서 꺼내자마자 충돌이 켜지므로 `InitGroundEffect` 호출 전에 이미 Overlap 이벤트 발생 가능. 에너미가 스폰 위치 근처에 있으면 InitGroundEffect 이전에 ApplyGEToTarget 호출 → 캐시 null → 조용한 실패.
  - B) `InitGroundEffect` 마지막 줄에서 충돌 활성화 — 모든 캐시 설정 완료 후 충돌을 열므로 순서 보장.

**결정**: B안 채택. `OnPoolActivate`에서는 충돌을 비활성화하고, `InitGroundEffect` 끝에서만 활성화.

**결과/효과**: "필수 데이터 누락" 로그 사라짐. 장판이 올바르게 GE를 적용.

**포트폴리오 포인트**: 풀링 패턴에서 초기화 순서(Activate → Init)와 이벤트 트리거 타이밍의 경합 조건 진단. GAS 컴포넌트의 생명주기와 UE 충돌 시스템의 상호작용 이해.

**관련 파일**: Source/RoastStaffGAS/Private/Objects/GroundEffect/GroundEffectActor.cpp

**검증 기준**:
  - [x] 장판 스폰 시 "필수 데이터 누락" 로그 미출력
  - [x] 에너미가 장판 위에 있어도 초기화 전 GE 미적용

---

### [2026-04-21] [PATTERN] PostProcess 아웃라인 — CustomDepth 깊이차 비교 vs CustomStencil 이진 마스크

**UE_Ver**: 5.4
**Knowledge_Risk**: LOW

**상황**: 로비 캐릭터 선택 호버 시 외곽선을 표시하기 위해 PostProcess 머티리얼에서 `SceneTexture: CustomDepth`를 사용해 인접 픽셀 간 깊이 차이로 엣지를 감지하는 방식을 구현했다.

**문제/과제**: 캐릭터 메시에 커서를 올리면 외곽선만 빛나야 하는데 **메시 전체가 빛나는 현상** 발생. 원인 파악이 필요했다.

**검토한 선택지**:
- A) `CustomDepth` 깊이차 비교 (기존 구현)
  ```hlsl
  float edge = step(0.01, abs(Center - Right)) + ...;
  ```
  문제: CustomDepth는 카메라로부터의 월드 거리(실수)를 저장. 메시 **내부**에서도 인접 픽셀 간 깊이가 다르기 때문에(예: 팔 200, 몸통 205 units) `abs diff > 0.01` 조건이 내부 전체에서 참이 됨 → 메시 전체 발광.

- B) `CustomStencil` 이진 마스크 비교
  ```hlsl
  float IsCenter = step(0.5, Center); // 1=메시, 0=배경
  float edge = IsCenter * saturate((1-IsRight) + (1-IsLeft) + (1-IsDown) + (1-IsUp));
  ```
  C++에서 `SetCustomDepthStencilValue(1)` 호출. Stencil은 "메시 있음(1) / 없음(0)" 이진값이므로 내부 깊이 변화에 영향받지 않음.

**결정**: B안 채택. SceneTexture 노드 5개를 `CustomDepth` → `CustomStencil`로 교체하고 HLSL을 이진 비교로 변경.

**결과/효과**: 메시 외곽선만 정확하게 발광. 내부는 발광 없음.

**포트폴리오 포인트**: UE5 PostProcess 머티리얼에서 커스텀 깊이 버퍼의 두 채널(Depth: 실수 깊이 / Stencil: 이진 마스크) 특성 차이 이해. 화면공간 엣지 감지 알고리즘 구현 경험.

**관련 파일**: Content/PostProcess/M_LobbyOutline (머티리얼 에셋), LobbyCharacterActor.cpp

**검증 기준**:
  - [x] 커서 호버 시 메시 외곽선만 발광, 내부는 발광 없음
  - [x] `SetRenderCustomDepth(false)` 시 아웃라인 완전 소멸

---

### [2026-04-20] [ARCH] CC 시스템 설계 — GE GrantedTags vs SetByCaller float 인코딩

**UE_Ver**: 5.4
**Knowledge_Risk**: LOW

**상황**: AoE 스킬에 넉다운 CC를 추가하면서 GE가 "어떤 CC를 적용할지"를 EnemyAttributeSet에 전달하는 방법이 필요했다.

**문제/과제**: GAS의 GE는 데미지 수치(SetByCaller)는 전달할 수 있지만, "넉다운이냐 스턴이냐"는 열거형 정보를 전달하는 공식 채널이 명확하지 않았다.

**검토한 선택지**:
  - A) SetByCaller float 인코딩 — CC 종류를 1.0(넉다운), 2.0(스턴) 등으로 인코딩. 구현이 단순하나 GE와 코드 간 암묵적 계약이 생기고 태그 기반 GAS 철학에 반함.
  - B) GE GrantedTags에 CC 태그 부여 → PostGameplayEffectExecute에서 `GetAllGrantedTags()`로 읽어 분기 — GAS 설계 원칙에 부합, GE BP에서 태그만 추가하면 새 CC 종류 확장 가능.

**결정**: B안 채택. `CC.Knockdown` / `CC.Stun` / `CC.Blind` 태그를 네이티브 태그로 등록하고 `GetAllGrantedTags()`로 읽어 분기. Instant GE에서 `GrantedTags`가 ASC에 유지되지 않는 문제는 `GetAllGrantedTags()`가 실행 시점 스펙에서 직접 읽으므로 무관.

**결과/효과**: GE BP에서 태그 하나 추가로 CC 종류 지정 가능. C++ 코드 수정 없이 GE 레벨에서 CC 조합 가능한 확장성 확보.

**포트폴리오 포인트**: GAS GrantedTags의 Instant vs HasDuration 동작 차이 이해. GE Context(HitResult ImpactPoint)를 통한 AoE Center 기반 넉백 방향 계산 패턴.

**관련 파일**: Source/RoastStaffGAS/Private/GAS/Attributes/EnemyAttributeSet.cpp, Source/RoastStaffGAS/Private/GAS/Abilities/GA_CharacterSkill.cpp

**검증 기준**:
  - [x] CC.Knockdown 태그 GE 적용 시 ApplyKnockdown 호출
  - [x] CC 태그 없는 GE 적용 시 ApplyHitReact 호출
  - [x] AoE Center 기준 넉백 방향 정확 (에너미가 Center에서 멀어지는 방향)

### [2026-04-21] [ARCH] SpawnPreview 공유 BP 상태 오염 — 스킬별 GA BP 분리 결정

**UE_Ver**: 5.4
**Knowledge_Risk**: LOW

**상황**: SpawnPreview ActivationType을 스킬 3(텔레포트)과 스킬 5(장판 소환) 양쪽에 사용하면서 `bTeleportOnConfirm` 플래그로 동작을 분기하려 했다. 스킬 5가 SpawnPreview 타입으로 변경되자 예상치 못하게 텔레포트 동작을 했다.

**문제/과제**: 원인 조사 결과, 공유 BP(`GA_CharacterSkill_SpawnPreview`)에 `FXActorClass`가 설정되어 있었고, 이전 코드에서 ExecuteSpawnPreview가 `FXActorClass` 존재 여부와 무관하게 항상 텔레포트를 수행하는 구조였음. `bTeleportOnConfirm` 플래그를 추가해도 `EditDefaultsOnly` 속성은 BP 클래스 단위로 공유되므로, 플래그를 true로 설정하면 해당 BP를 사용하는 **모든 스킬**이 텔레포트하게 됨.

**검토한 선택지**:
  - A) 공유 BP에 `bTeleportOnConfirm` 플래그 추가 후 분기 — 플래그 자체는 맞는 설계지만, BP가 공유되는 한 스킬별로 다른 값을 가질 수 없음. 결국 동작이 다른 스킬마다 별도 BP가 필요해 근본 해결이 아님.
  - B) 스킬별 GA BP 분리 (Painter03, Painter05 전용 BP) — EditDefaultsOnly 속성을 스킬별로 독립적으로 설정 가능. BP 수가 늘어나는 단점은 있으나, 이는 곧 GA/Actor 레이어 구조 재설계(PLAN_SkillSystemArch)로 해소 예정.

**결정**: B안. 스킬별 GA BP를 분리하고, 추후 `SkillGEClass`를 DataTable로 이동해 GA BP를 ActivationType 단위로만 유지하는 구조 리팩토링을 별도 PLAN으로 잡기로 결정.

**결과/효과**: Painter03(텔레포트, bTeleportOnConfirm=true), Painter05(장판, false)가 각각 독립 동작. 동시에 아키텍처 문제가 구체적으로 드러나 PLAN_SkillSystemArch_v1.0 착수 근거 확보.

**포트폴리오 포인트**: UE5 `EditDefaultsOnly`가 BP 클래스 인스턴스가 아닌 클래스 단위로 값을 공유함을 간과한 버그 케이스. "같은 타입이면 BP 공유 가능"이라는 가정이 깨질 때 발생하는 구조적 문제와, 이를 계기로 시스템 레이어 설계를 재검토한 의사결정 과정.

**관련 파일**:
  - Source/RoastStaffGAS/Public/GAS/Abilities/GA_CharacterSkill.h (bTeleportOnConfirm)
  - Source/RoastStaffGAS/Private/GAS/Abilities/GA_CharacterSkill.cpp (ExecuteSpawnPreview)
  - Content/GAS/GA/Character/Painter/ (GA BP 분리)

**검증 기준**:
  - [x] 스킬 3 발동 시 텔레포트, 스킬 5 발동 시 텔레포트 없이 장판 소환
  - [x] 소서리스/호크아이 SpawnPreview 스킬 텔레포트 없음

---

### [2026-04-18] [ARCH] 캐릭터 스킬 ProjectileSpawn — DataTable 신규 컬럼 vs SkillEffectID FK 재사용

**UE_Ver**: 5.4
**Knowledge_Risk**: LOW

**상황**: 캐릭터 고유 스킬(Q/E)에 ProjectileSpawn 타입을 추가하면서 투사체 파라미터(속도, 수명, 관통 수 등)를 어디에 저장할지 결정해야 했다.

**문제/과제**: `FCharacterSkillStaticData`에 ProjectileClass, Speed, PierceCount 등 파라미터를 직접 추가하는 초기 구현이 이미 존재하는 무기 스킬 테이블(DT_Skill_Attack_Common_Params_Data, DT_Skill_Attack_HitType_Params_Pierce 등)과 동일한 데이터를 중복 정의하는 구조가 됨. 같은 수치를 두 곳에서 관리하면 수정 시 불일치 발생 위험.

**검토한 선택지**:
  - A) `FCharacterSkillStaticData`에 직접 파라미터 추가 (초기 구현) — 구현이 단순하나 무기 스킬 테이블과 데이터 중복. 수치 변경 시 두 테이블 동기화 필요. 나중에 타입별 파라미터가 늘어날수록 구조체가 비대해짐.
  - B) `SkillEffectID` FK를 추가하고 GDS 복합 조회로 기존 테이블 재사용 — FK 하나만 추가하면 무기 스킬 인프라(AttackCommonParams, SpawnParams, HitTypePierce) 전체를 공유. 단, 캐릭터 스킬 SkillID로 DT_Skill_Common_Resource_Data에 row도 추가해야 한다는 에디터 설정 부담 존재.

**결정**: Option B 선택. DRY 원칙 우선 — 수치가 두 곳에 있으면 버그 재현이 어렵고 DataTable 유지보수 비용이 높아진다. 에디터 설정 부담은 개발 초기에 수용 가능한 수준.

**결과/효과**: `FCharacterSkillStaticData`에 `SkillEffectID`와 `FireInterval`(캐릭터 스킬 전용 burst 파라미터) 2개만 추가. `GetCharacterSkillExecData`가 SkillEffectID를 통해 기존 캐시 5개(Resource, AttackCommon, CommonParam, SpawnParams, HitTypePierce)를 복합 조회해 `FCharacterSkillExecData`를 완성. 하드코딩 10.f도 `AttackCommonParams.Amount`로 대체되어 제거.

**포트폴리오 포인트**: 데이터 중복 vs FK 재사용 트레이드오프 인식 / 기존 스키마를 최소 변경으로 확장하는 OCP 적용 / GDS 복합 조회 패턴 설계

**관련 파일**:
  - Source/RoastStaffGAS/Public/Data/DataTableStructs.h (FCharacterSkillStaticData)
  - Source/RoastStaffGAS/Public/Data/RuntimeDataStructs.h (FCharacterSkillExecData)
  - Source/RoastStaffGAS/Private/Subsystems/GameDataSubsystem.cpp (GetCharacterSkillExecData)

**검증 기준**:
  - [x] 빌드 성공 (2026-04-18)
  - [ ] ProjectileSpawn 타입 스킬 발동 시 DT row 설정대로 투사체 발사 확인

---

### [2026-04-14] [BUG_FIX] LoadingWidget Dangling Pointer — 레벨 전환 시 UIManagerSubsystem 캐시 불일치

**상황**: InGame 진입 후 LoadingWidget이 PreWarm 완료 전에 닫히고, 첫 무기 슬롯 등록이 지연되는 버그. `FinishLoading()` 호출 시점에 `IsVisible: 0` 로그 발견.

**문제/과제**: 레벨 전환(TransitionGameMode → RSGameMode) 시 Widget의 World가 파괴되면서 Widget 자체도 파괴됨. 그러나 GameInstanceSubsystem인 UIManagerSubsystem은 레벨 간 유지되어 `bIsOpen=true` + dangling pointer 상태 잔존. 새 World에서 `OpenUIByID` 호출 시 `bIsOpen=true` 조기 반환으로 `AddToViewport` 스킵 → Widget은 메모리에만 있고 Viewport에는 없는 상태.

**검토한 선택지**:
  - A) LOADING UI를 PAGE → SYSTEM 레이어로 변경 — 레이어 변경으로는 World 파괴 문제 미해결
  - B) RSGameMode::BeginPlay에서 LoadingWidget 강제 재생성 — 레벨마다 새 Widget 생성, 깜빡임 우려
  - C) dangling 상태 감지 후 CloseUI → OpenUI — `IsOpen() && !IsInViewport()` 조건으로 정확히 식별

**결정**: C 선택. `BeginPlay`에서 `GetWidgetByID` 조회 후 dangling 상태 확인, `CloseUIByID`로 `bIsOpen` 플래그 리셋 후 `OpenUIByID` 재호출로 `AddToViewport` 보장.

**결과**: LoadingWidget이 RSGameMode BeginPlay 시점에 정상적으로 Viewport에 추가되고, PreWarm 진행률 갱신 및 FinishLoading 정상 동작 확인. 레벨 전환 간 UI 상태 불일치 해소.

**포트폴리오 포인트**: UMG Widget의 World 소유권과 GameInstanceSubsystem의 생명주기 차이 이해 / Widget이 파괴된 후 캐시만 남은 dangling 상태를 `IsInViewport()` 조건으로 식별하는 방어적 초기화 패턴 적용

**관련 파일**: `Source/RoastStaffGAS/Private/Core/RSGameMode.cpp:54-61`, `Source/RoastStaffGAS/Private/UI/Transition/RSLoadingWidget.cpp`

---

### [2026-04-14] [ARCH] 레벨업 카드 선택 로직 — Widget 직접 처리 vs LevelUpSubsystem 위임

**상황**: MODULE-6에서 레벨업 카드를 무기 전용에서 스탯/패시브/무기 혼합으로 확장. 기존에는 Widget이 카드 선택 시 `EquipSys->EquipWeapon()`을 직접 호출하는 구조.

**문제/과제**: 카드 타입이 4종(StatUpgrade / PassiveAdd / WeaponUpgrade / WeaponNew)으로 늘어나면서 각 타입마다 다른 서브시스템을 호출해야 함. Widget이 이 분기를 직접 처리하면 Widget이 게임 로직을 알아야 하는 구조가 됨.

**검토한 선택지**:
  - A) Widget에서 CardType 분기 후 각 서브시스템 직접 호출 — 간단하지만 Widget이 EquipSys/PassiveSys/ASC를 알아야 함. 카드 타입 추가 시 Widget 수정 필요
  - B) Widget은 CardID만 전달, LevelUpSubsystem이 타입 조회 후 처리 — Widget은 "무엇인지 모르고 ID만 던짐". 카드 타입 추가 시 Subsystem만 수정

**결정**: B 선택. `LevelUpSubsystem::OnCardSelected(CardID)`가 GDS 조회로 타입 판별 후 StatUpgrade→ASC 직접 적용 / PassiveAdd→PassiveSlotSubsystem / 무기→EquipmentSubsystem으로 분기.

**결과**: Widget이 게임 로직 의존성 제로. 카드 타입 추가 시 LevelUpSubsystem만 수정. TDA(Tell Don't Ask) 원칙 적용 — Widget이 상태를 물어보지 않고 서브시스템에게 위임.

**포트폴리오 포인트**: UI-Logic 역할 분리 / TDA 원칙 적용으로 확장성 확보 / 단일 책임 원칙

**관련 파일**: `Source/RoastStaffGAS/Private/Subsystems/LevelUpSubsystem.cpp:385`, `Source/RoastStaffGAS/Private/UI/LevelUpWeaponSelectWidget.cpp:144`

---

### [2026-04-14] [PATTERN] GameInstanceSubsystem에서 WorldSubsystem 접근

**상황**: `ULevelUpSubsystem`(GameInstanceSubsystem)이 `UPassiveSlotSubsystem`(WorldSubsystem)에 접근해야 함.

**문제/과제**: `UGameInstanceSubsystem`은 `UObject` 기반 → `GetWorld()` 직접 호출 불가한 것처럼 보임. `GetGameInstance()->GetWorld()->GetSubsystem<>()` 수동 작성.

**검토한 선택지**:
  - A) `GetGameInstance()->GetWorld()->GetSubsystem<>()` 수동 — 장황, null 체크 필요
  - B) `GET_WORLD_SUBSYSTEM` 매크로 — 프로젝트 표준이나 GameInstanceSubsystem에서 동작 여부 불확실

**결정**: B 확인 후 적용. `UGameInstanceSubsystem::GetWorld()`가 내부적으로 `GetGameInstance()->GetWorld()`를 반환하도록 오버라이드되어 있어 매크로가 정상 동작.

**결과**: 프로젝트 전체 매크로 일관성 유지. linter가 수동 코드를 매크로로 자동 교체.

**포트폴리오 포인트**: UE5 서브시스템 생명주기 이해 / GameInstance ↔ World 계층 구조 파악

**관련 파일**: `Source/RoastStaffGAS/Private/Subsystems/LevelUpSubsystem.cpp:177`

---

### [2026-04-14] [BUG_FIX] AutoFire 즉시 발사 — PreWarm 중 타이머 등록 + SetTimer 초기 딜레이 0초 문제

**상황**: PreWarm 완료 직후 보스가 스폰되자마자 무기 스킬이 즉시 발사되고 EndAbility 로그가 찍히는 버그. UI에는 슬롯이 표시되기 전에 스킬이 이미 소진됨.

**문제/과제**:
1. `EquipmentSubsystem::CommitSlot`에서 `StartAutoFire` 즉시 호출 → PreWarm 중에도 타이머 등록됨 → 적이 없어 타겟 없이 발사 스킵
2. `SetTimer`의 초기 딜레이 파라미터가 `0.f`로 설정되어 타이머 등록 즉시 첫 발사 실행

**검토한 선택지**:
  - A) StartAutoFire를 private→public으로 이동, StartStageFlow에서 재호출 — API 노출 증가, 복잡도 상승
  - B) `ARSGameMode::bIsPreWarmActive` public 플래그 추가, StartAutoFire에서 조기 리턴 — 단순 명확
  - C) SetTimer 초기 딜레이를 Cooldown 값으로 변경 — 첫 발사가 Cooldown만큼 지연(8초 무기는 8초 대기)

**결정**: B + 고정 딜레이 상수.
  - B안 채택: `bIsPreWarmActive` 체크로 PreWarm 중 타이머 등록 스킵
  - 초기 딜레이: `AUTO_FIRE_START_DELAY = 5.f` 상수 추가, Cooldown과 무관하게 고정 5초 후 첫 발사

**결과**: PreWarm 중에는 타이머 미등록, 완료 후 첫 무기 장착 시 타이머 등록, 5초 후 첫 발사. 보스 스폰(~1초) + 고정 딜레이(5초) = 총 6초 후 자동 공격 시작으로 자연스러운 전투 시작 타이밍 확보.

**포트폴리오 포인트**: SetTimer의 초기 딜레이 파라미터 활용 / GameMode 상태 플래그를 통한 Subsystem 간 동기화 / 사용자 피드백("노. 아니야 이 방식은 잘못됐어")을 받아 즉시 롤백하고 더 단순한 해법 채택한 협업 사례

**관련 파일**: `Source/RoastStaffGAS/Public/Subsystems/EquipmentSubsystem.h:95`, `Private/Subsystems/EquipmentSubsystem.cpp:292-309`, `Public/Core/RSGameMode.h:77`

---

### [2026-04-13] [ARCH] SpawnPreview 액터 — GameMode 전역 단일 클래스 → DT 스킬별 분리

**상황**: SpawnPreview 타입 캐릭터 스킬이 구현됐지만, 모든 캐릭터의 프리뷰가 동일한 액터(GameMode.PreviewActorClass)를 사용. 캐릭터마다 다른 형태의 프리뷰(범위 표시, 투사체 방향 표시 등)가 필요함
**문제**: GameMode에 단일 `TSubclassOf<ASummonPreviewObject> PreviewActorClass` UPROPERTY를 두고 SkillManagerSubsystem이 이를 참조하는 구조. 스킬이 늘어날수록 분기 처리가 GameMode에 누적되는 구조적 결함
**검토한 선택지**:
  - A) `DT_CharacterSkill`에 `PreviewActorClass (TSoftClassPtr<ASummonPreviewObject>)` 추가 — 완전 데이터 드리븐, GameMode 의존성 0
  - B) `GA_CharacterSkill` 서브클래스화 — 캐릭터마다 BP_GA 별도 제작. 에디터 파일 증가, 스킬 추가 시 비용 큼
**결정**: A 선택. `CSV→DataTable→Subsystem→GA` 원칙에 완벽히 부합. GameMode의 `PreviewActorClass` UPROPERTY·getter 제거. `PreviewFXClass(TSoftClassPtr<UNiagaraSystem>)` 도 제거 — PreviewActor BP가 자체 FX 컴포넌트를 보유하므로 중복
**결과**: SkillManagerSubsystem이 `ExecData.PreviewActorClass.LoadSynchronous()`로 스킬별 액터 스폰. 미설정 시 `ASummonPreviewObject::StaticClass()` 폴백
**포트폴리오 포인트**: 데이터 드리븐 아키텍처 원칙을 지키기 위해 GameMode 전역 상태를 DT 행 단위로 분해한 설계 판단
**관련 파일**: `DataTableStructs.h:695`, `SkillManagerSubsystem.cpp:192-220`, `RSGameMode.h`

---

### [2026-04-13] [ARCH] GA_CharacterSkill FX 스폰 — Niagara Radius 파라미터 주입

**상황**: InstantAoE·SelfBuff·SpawnPreview 모두 GE 적용(데미지/버프)만 하고 시각 피드백이 없었음. `FCharacterSkillLevelData.FXClass`는 정의돼 있었으나 GA에서 미사용
**문제**: `FXClass`가 `TSoftClassPtr<UNiagaraSystem>`으로 선언돼 있어 에디터 피커에서 에셋이 표시되지 않는 타입 오류
**결정**: `TSoftObjectPtr<UNiagaraSystem>`으로 수정(에셋 참조). `SpawnSkillFX(FXClass, Location, Radius)` 헬퍼로 공통화. Niagara 컴포넌트에 `SetVariableFloat("Radius", Radius)` 주입으로 이펙트 크기를 데이터로 제어
**결과**: AoE·SelfBuff → 캐릭터 위치, SpawnPreview → 확정 위치에 FX 스폰. Niagara 시스템 내부에서 Radius User Parameter를 읽어 이펙트 범위 설정
**포트폴리오 포인트**: TSoftClassPtr/TSoftObjectPtr 구분(클래스 vs 에셋), Niagara User Parameter 런타임 주입 패턴
**관련 파일**: `GA_CharacterSkill.cpp:195-215`, `DataTableStructs.h:664`

---

### [2026-04-13] [BUG_FIX] EquipmentSubsystem 재진입 시 무기 슬롯 미등록

**상황**: GameInstanceSubsystem은 레벨 전환에도 유지됨. InGame → Lobby → InGame 재진입 시 시작 무기가 Slot에 등록되지 않음
**문제**: `DeinitializeSubsystem()`이 `ASC`, `bIsInitialized`만 리셋하고 `Slots[]` 배열은 초기화 안 함 → 재진입 시 이전 세션의 WeaponID 잔존 → `IsEmpty()=false` → `GetEmptySlotIndex()=INDEX_NONE` → `OnSlotFull` 오발동
**검토한 선택지**:
  - A) `DeinitializeSubsystem`에서 Slots[] 초기화 — 해제 시점에 처리
  - B) `InitializeSubsystem`에서 Slots[] 완전 초기화 후 SlotIndex 재설정 — 진입 시점에 방어
**결정**: B 선택. 초기화 시점 방어가 더 명확한 진입점 보장. `Slots[i] = FWeaponSlotInstanceData()` 후 `Slots[i].SlotIndex = i`
**결과**: 재진입 시 슬롯 정상 등록 확인
**포트폴리오 포인트**: GameInstanceSubsystem 생명주기와 레벨 전환 간 상태 잔존 문제 식별 및 방어적 초기화 적용
**관련 파일**: `Source/.../EquipmentSubsystem.cpp:30-34`

---

### [2026-04-13] [BUG_FIX] SpawnPreview EffectRadius=0 → 반경 1cm 오버랩으로 데미지 미적용

**상황**: CHAR_ROGUE/MAGE의 SpawnPreview 스킬이 첫 확정 시는 데미지가 들어가나 이후 전혀 안 됨
**문제**: DT_CharacterSkill의 SpawnPreview 행에 `EffectRadius=0` 입력. `ExecuteSpawnPreview`에서 `FMath::Max(1.f, 0.f)` → 반경 1cm 구체로 오버랩 → 적 미탐지. 첫 번째 동작한 이유는 플레이어가 적에 매우 근접한 경우였음
**진단 과정**: TryActivateAbility 결과 로그 + OnAbilityActivated 진입 로그 추가 → GA는 정상 진입 → OverlapMultiByChannel 반환값 0 확인 → Radius 추적 → EffectRadius=0 발견
**결정**: DT에서 SpawnPreview 행에 EffectRadius 값 직접 입력하여 해결 (C++ 변경 없음)
**교훈**: `FMath::Max(1.f, x)` 패턴은 0 입력 시 사실상 "무반응"으로 침묵하는 버그. DT 필드가 실제 코드에서 어떻게 쓰이는지 설명 전 코드 확인 필수
**포트폴리오 포인트**: GAS 능력 활성화 단계별 로그를 직접 삽입해 레이어별로 좁혀가는 디버깅 방법론 적용
**관련 파일**: `Source/.../GA_CharacterSkill.cpp:136`, `ExternalSource/DT_Character_Skill_Static_Data.csv`

---

### [2026-04-13] [BUG_FIX] 스테이지 클리어 후 캐릭터 해금 미처리

**상황**: STG_003 클리어 후 CHAR_MAGE가 캐릭터 선택 화면에서 잠금 해제되지 않음
**문제**: `RSGameMode::SaveResult` → `SGS->UpdateStageRecord()` 내부에서 `ClearedStageIDs` 갱신은 되나, `SGS->UnlockCharacter()` 호출 경로가 코드 어디에도 없음. DT에 `UnlockType=STAGE_CLEAR`, `UnlockStageID=STG_003` 데이터가 올바르게 있었음에도 미연결
**결정**: `SaveResult`에서 `bCleared=true` 시 `GDS->GetAllCharacterStaticData()`로 전체 캐릭터 조회 → `UnlockType==STAGE_CLEAR && UnlockStageID==CurrentStageID` 조건 캐릭터에 `SGS->UnlockCharacter()` 호출. `UpdateStageRecord` 전에 처리해 해금 데이터가 동일 `SaveGame()` 호출에 포함되도록
**결과**: 클리어 시 해당 캐릭터 즉시 해금 및 디스크 저장 확인
**포트폴리오 포인트**: 데이터 저장과 파생 상태 갱신의 원자성 — 해금 처리를 UpdateStageRecord(SaveGame 포함) 이전에 배치해 단일 저장 호출로 묶음
**관련 파일**: `Source/.../RSGameMode.cpp:364-390`, `Source/.../SaveGameSubsystem.cpp:134`

---

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

## [2026-04-20] PATTERN — 머티리얼 피격 플래시: Multiply vs Add 블렌딩 선택

**UE_Ver**: 5.4
**Knowledge_Risk**: LOW

**상황**: 에너미 피격 시 이미시브 붉은 플래시 연출 구현. `EmissiveIntensity` ScalarParameter를 C++에서 0→3으로 올리는 방식 채택. 머티리얼 그래프에서 `Texture_Emissive × EmissiveIntensity × Constant3Vector(1,0.3,0.3)` 구조로 연결했으나 색상 변화 없음.

**문제·과제**: EmissiveIntensity를 3으로 올려도 빨갛게 변하지 않음. Multiply 연산에서 텍스처 고유 색상 채널이 Constant3Vector를 지배하기 때문. 예: 텍스처 픽셀 `(0.5, 0.7, 0.9)` × `(1, 0.3, 0.3)` = `(0.5, 0.21, 0.27)` → 의도한 붉은빛이 아닌 탁한 색.

**검토한 선택지**:
- A) Multiply 유지 + Constant3Vector 값을 강하게 올림 → 원본 텍스처 색상이 과하게 억제됨
- B) Lerp(TextureEmissive, FlashColor, EmissiveIntensity) → 평소 이미시브가 사라지지 않지만 Saturate 처리 필요, 복잡도 증가
- C) **Add** — 기존 텍스처 이미시브 유지 + 플래시 컬러를 additive로 합산

**결정**: Add 구조 채택. `[Texture_Emissive] + [Constant3Vector × EmissiveIntensity] → Emissive Color`. EmissiveIntensity=0일 때 원본 이미시브 그대로, 피격 시 붉은빛이 텍스처 색상과 무관하게 덧씌워짐.

**결과**: 모든 에너미 머티리얼에 동일 구조 적용. C++ 코드 변경 없이 일관된 붉은 플래시 연출 확보.

**포트폴리오 포인트**: 머티리얼 그래프에서 Multiply는 "색조 필터", Add는 "발광 덧씌우기"로 동작이 근본적으로 다름. 피격·버프·상태이상처럼 원본 색상과 무관하게 일관된 이미시브를 올려야 할 때는 Add가 정답. Multiply는 텍스처의 밝기/색조를 바꿀 때 사용.

**관련 파일**: `M_HighDemon_Skin1/3`, `M_Night_Demon_Armor/Body`, `M_Skeleton_Guard_Body/Cloth`, `M_SpiderQueen`, `EnemyBaseCharacter.cpp`

---

## [2026-04-15] ARCH — UMG 툴팁 컨테이너: UBorder 대신 UOverlay 선택

**상황**: PassiveSlotWidget 툴팁 컨테이너를 초기 설계에서 UBorder로 지정. C++ 코드 작성 후 WBP 제작 단계에서 문제 발견.

**문제·과제**: Canvas Panel 위에 배치된 UBorder는 내부 TextBlock 크기에 맞게 자동 리사이징이 되지 않음. Canvas Panel 슬롯의 크기가 고정 좌표/크기로 잡히기 때문이며, Content Alignment를 Fill로 두면 부모 공간을 꽉 채워 TextBlock이 짧아도 Border가 늘어난 채로 유지됨.

**검토한 선택지**:
- A) VerticalBox/HBox 로 감싸서 Auto 슬롯 사이즈 활용 — 위젯 계층 한 단계 추가
- B) UOverlay 로 교체 — 자식 크기에 자연스럽게 맞고, Image(배경) + VerticalBox(텍스트) 레이어링도 동시에 해결

**결정**: UOverlay 채택. BindWidget 이름을 `Bdr_Tooltip → Ovl_Tooltip`으로 변경하고 C++ include도 `Border.h → Overlay.h`로 교체. WBP 계층 구조: Ovl_Tooltip(Overlay) > Image(배경) + VerticalBox > Txt_PassiveName / Txt_PassiveDesc.

**결과**: 툴팁이 내부 텍스트 길이에 맞게 자동 리사이징. Image 배경이 텍스트 영역에 정확히 맞춰짐.

**포트폴리오 포인트**: UMG 패널별 크기 결정 방식 차이를 실제 제작 중 발견하고 적용 — Canvas Panel 슬롯은 고정, Box/Overlay는 자식 크기(Auto)를 따름. 설계 단계에서 WBP 레이아웃 의존성을 미리 고려해야 C++ BindWidget 타입 선택이 흔들리지 않는다는 교훈.

**관련 파일**: `PassiveSlotWidget.h/.cpp`, `WBP_PassiveSlot`

---

## [2026-04-15] BUG_FIX — GAS Multiplicative modifier 공식 오진 → 진단 로그로 재확인

**UE_Ver**: 5.4
**Knowledge_Risk**: MEDIUM

**상황**: 패시브 GE Magnitude가 1.05~1.20이었는데 유저가 "스탯 증가폭이 의도보다 크다"고 보고. 공식을 `Base × (1 + Sum(mags))`로 분석해 magnitude를 0.05~0.20으로 변경했으나, 이후 DEF 15 → 3 (ICE_ARMOR_4, 0.20 적용)으로 오히려 대폭 감소하는 버그가 발생함.

**문제·과제**: C++ ApplyStatUpgrade 로직은 이상 없고, GE BP 설정도 시각적으로 확인 어려운 상황. 실제 GAS aggregator가 magnitude를 어떻게 처리하는지 런타임에서 직접 검증 필요.

**검토한 선택지**:
1. GE BP의 modifier 타입 확인 (Override vs Multiplicative)
2. 진단 로그 삽입으로 `GetNumericAttributeBase` vs `GetNumericAttribute` 동시 출력

**결정**: PassiveSlotSubsystem의 GE 적용 전/후에 `DEF_base`, `DEF_cur`, `ATK_cur`를 동시 로깅. 결과: `DEF_base=15.00, DEF_cur=3.00` → `15 × 0.20 = 3.00` 확인. GAS Multiplicative는 `Base × Magnitude` (Product 방식)이며, 분모에 1이 없음. magnitude 1.0x 원복.

**결과**: 패시브 ATK +20% = `Base × 1.20`, DEF +5% = `Base × 1.05`. StatUpgrade +20은 `SetNumericAttributeBase(base + 20)` 이후 aggregator 재계산 → `NewBase × 1.20`. 의도한 동작 확인.

**포트폴리오 포인트**: GAS Infinite GE의 Multiplicative modifier는 `Product(mags)` 방식 (`Base × mod1 × mod2 × ...`). 빈 상태는 1.0이 아닌 0으로 시작하지 않고, modifier 없으면 최종값 = Base 그대로. 공식 가정 전 `GetNumericAttributeBase` / `GetNumericAttribute` 쌍으로 aggregator 동작을 가시화하는 것이 가장 빠른 디버깅 패턴.

**관련 파일**: `ExternalSource/DT_Passive_Static_Data.csv`, `PassiveSlotSubsystem.cpp`, `LevelUpSubsystem.cpp`

---

## [2026-04-15] REFACTOR — 커스텀 AS 델리게이트 제거 → ASC GetGameplayAttributeValueChangeDelegate 일원화

**UE_Ver**: 5.4
**Knowledge_Risk**: LOW

**상황**: CharacterStatPopupWidget이 `PostGameplayEffectExecute` 기반의 `OnPlayerStatChangedDel`, `OnHealthChangedDel`, `OnMoveSpeedChangedDel` 커스텀 델리게이트를 구독. StatUpgrade 카드 선택 후 위젯이 갱신되지 않는 버그 발생.

**문제·과제**: `PostGameplayEffectExecute`는 Instant GE 실행 시에만 호출됨. `SetNumericAttributeBase` 경유 변경(StatUpgrade, LevelUp 시 MaxHP 갱신 등)은 감지 불가. 커스텀 델리게이트 3개가 모두 동일한 한계를 가짐.

**검토한 선택지**:
1. 수동 브로드캐스트 추가 (ApplyStatUpgrade에서 델리게이트 직접 호출)
2. ASC `GetGameplayAttributeValueChangeDelegate(Attr)` 구독 (GAS-idiomatic)

**결정**: 선택지 2 채택. 변경 경로(GE/SetNumericAttributeBase 등) 무관하게 자동 감지. 8개 어트리뷰트 모두 단일 핸들러 `OnStatChanged` → `RefreshAllStats()`로 일원화. BossEnemy HP 페이즈 전환도 동일 패턴으로 전환, `OnHealthChangedDel` 완전 제거.

**결과**: 커스텀 델리게이트 3종 제거, AttributeSet 코드 간소화. StatUpgrade/PassiveAdd/LevelUp 모든 경로에서 위젯 즉시 갱신.

**포트폴리오 포인트**: GAS에서 어트리뷰트 변경 알림은 `GetGameplayAttributeValueChangeDelegate(Attr).AddUObject()`가 idiomatic. `PostGameplayEffectExecute`는 GE 실행에만 한정. CloseUI 시 `RemoveAll(this)`로 정리하면 생명주기 안전.

**관련 파일**: `BaseAttributeSet.h/cpp`, `PlayerAttributeSet.h/cpp`, `BossEnemy.h/cpp`, `CharacterStatPopupWidget.h/cpp`

---

## [2026-04-14] ARCH — 인게임 스탯 팝업의 UMS 등록 제외 결정

**상황**: 캐릭터 어트리뷰트를 실시간으로 표시하는 팝업 UI(WBP_CharacterStatPopup)를 설계. HUD 버튼 + Tab 키로 토글되며, 게임 입력을 차단하지 않아야 함.

**문제·과제**: UMS(UIManagerSubsystem)를 통해 일반 POPUP 레이어로 등록하면 `OpenUIByID` 호출 시 `NotifyInputModeChange`가 트리거되어 GameOnly → UIOnly 전환이 발생함. 이는 "팝업 열린 채 이동/공격 가능" 요구사항과 충돌.

**검토한 선택지**:
- A) EUIID에 `CHAR_STAT_POPUP` 등록 + `bIsModal=false`로 등록 → UMS 스택 진입 시 입력 모드 변경 로직을 조건부 우회해야 함. UMS 내부 수정 필요, 부작용 범위 불명확.
- B) `UILayer=NONE`으로 HUD 자식 위젯으로 구현, UMS 미등록 → HUD 생명주기와 동기화, 입력 모드 변경 없음. `EnemyHPBarWidget` 등 기존 "UMS 제외" 위젯 패턴과 일관성 유지.

**결정**: B 채택. `UILayer=NONE` + HUD BindWidget 자식으로 배치. 토글 로직은 `URSHUDWidget::ToggleStatPopup()`에 캡슐화하고, PC의 Tab 입력과 HUD 버튼 양쪽에서 동일 함수를 호출하는 단일 경로 구조로 정리.

**결과**: 팝업 열림 상태에서 무기 자동발사·스킬 Q/E 전부 정상 동작 확인. UMS 내부 수정 없이 요구사항 충족.

**포트폴리오 포인트**: UMS 아키텍처의 입력 모드 관리 흐름을 이해하고, 기능 추가 시 기존 시스템을 수정하지 않고 "제외 패턴"을 일관되게 적용한 설계 판단.

**관련 파일**: `Public/UI/Ingame/CharacterStatPopupWidget.h`, `Public/UI/RSHUDWidget.h`

---

## [2026-04-14] BUG_FIX — PC::BeginPlay vs Character::BeginPlay 타이밍 역전으로 인한 스킬 슬롯 UI 미갱신

**상황**
캐릭터 스킬 슬롯 UI를 새로 구현. `SkillManagerSubsystem::InitializeSkills()` 완료 시 `OnSkillSlotUpdatedDel`을 브로드캐스트하고, `PC::BeginPlay`에서 구독해 UI를 갱신하는 구조.

**문제**
게임 플레이 시 캐릭터 스킬 슬롯이 항상 빈 상태로 표시됨. 로그를 확인하면 `InitializeSkills()` 브로드캐스트는 정상 발생하지만 PC가 수신하지 못함.

**원인**
UE5의 `BeginPlay` 호출 순서는 Actor 스폰 순서에 의존하며 보장되지 않음. 레벨에 Character가 이미 배치된 경우 `Character::BeginPlay → InitializeSkills → Broadcast`가 `PC::BeginPlay → 구독` 보다 먼저 실행되어 브로드캐스트를 놓침.

**해결**
두 가지 방어 코드 추가:
1. `SlotContainerWidget::NativeConstruct()`에서 모든 슬롯에 `UpdateSlot(nullptr)` 호출 → 초기 상태를 Collapsed로 보장
2. `PC::BeginPlay`에서 HUD 오픈 직후 `RefreshSkillSlotUI(0~1)` 강제 호출 → 이미 초기화된 경우 즉시 UI 반영

**포트폴리오 포인트**
UE5의 BeginPlay 순서 비보장 특성. 이벤트 드리븐 초기화만 믿지 않고, "구독 후 현재 상태 풀링(pull)"을 병행하는 패턴이 안전. 이는 옵저버 패턴의 "missed notification" 문제에 대한 일반적 해법.

**관련 파일**
- `Private/Subsystems/SkillManagerSubsystem.cpp` — InitializeSkills Broadcast 추가
- `Private/UI/Ingame/SlotContainerWidget.cpp` — NativeConstruct 초기 Collapsed
- `Private/Character/Player/RSPlayerController.cpp` — BeginPlay force-refresh

---

## [2026-04-13] PATTERN — UGameplayEffectExecutionCalculation (ExecCalc) 구조와 Static Capture 패턴

**상황**: M-4에서 플레이어→에너미, 에너미→플레이어 데미지를 단일 GE에서 분기 처리해야 했다. 기존 방식은 Modifier 방향을 SetByCaller 음수값(−DamageValue)으로 직접 조작하는 간이 구조였다.

**문제·과제**: Modifier-only GE는 ATK/DEF 같은 Attribute를 공식 내에서 읽을 수 없다. 크리티컬, DEF 감산 등 복잡한 공식을 GE 내부에서 계산하려면 GAS가 공식 지원하는 `UGameplayEffectExecutionCalculation`이 필요.

**검토한 선택지**:
- A) Modifier Stack — SetByCaller 음수 직접 주입. GE 설정 단순, 공식 확장 불가
- B) MMC (Magnitude Modifier Calculation) — 단일 Attribute 계산. 다중 Attribute 조합 불가
- C) ExecCalc (`UGameplayEffectExecutionCalculation`) — 다중 Attribute 캡처 + 임의 공식 + 다중 Attribute 출력 가능. 구조 복잡하나 GAS 공식 패턴

**결정**: C 선택. 핵심 패턴 3가지:
1. `FDamageExecCaptures` 정적 구조체 — `DECLARE/DEFINE_ATTRIBUTE_CAPTUREDEF` 매크로로 캡처 정의. `bSnapshot=false`로 실행 시점 라이브 값 캡처.
2. `Execute_Implementation` 내 Source 팀 태그 분기 — `CapturedSourceTags.GetAggregatedTags()->HasTagExact(Team_Player)`로 플레이어/에너미 방향 판별.
3. `OutExecutionOutput.AddOutputModifier` — `EGameplayModOp::Additive`로 CurrentHP에 `-FinalDamage` 출력.

**결과**: 플레이어→에너미 `BaseDmg×(1+ATK/100)×CritMult`, 에너미→플레이어 `max(1, EnemyDmg-DEF)` 두 공식을 단일 ExecCalc 클래스에서 처리.

**포트폴리오 포인트**: GAS 데미지 공식 설계의 세 단계(Modifier→MMC→ExecCalc) 이해와 적합한 계층 선택 / `DECLARE_ATTRIBUTE_CAPTUREDEF` + static struct 패턴 실전 적용.

**관련 파일**: `Source/RoastStaffGAS/Public/GAS/Calculations/RS_DamageExecCalc.h`, `Private/GAS/Calculations/RS_DamageExecCalc.cpp`

---

## [2026-04-13] ARCH — SetByCaller 태그 분리: Data_Damage 단일 → WeaponBaseDamage/EnemyAttackDamage

**상황**: 기존 모든 데미지(플레이어 무기, 에너미)가 `Data.Damage` 단일 태그에 음수값으로 주입됐다. ExecCalc 도입 시 Source 방향을 구분해야 하는데, 단일 태그로는 ExecCalc 내부에서 "누가 보낸 데미지인가"를 태그 없이 판단해야 함.

**문제·과제**: ExecCalc에서 Source ASC 팀 태그로 분기는 가능하나, SetByCaller 값 의미(양수 BaseDmg vs 음수 Modifier)가 혼재해 공식에 혼란이 생김. ExecCalc 내부에서 `-음수`를 다시 양수로 뒤집는 이중 부정이 발생.

**결정**: 태그 분리 + 양수 전달 원칙 확립.
- 무기(플레이어): `Data.WeaponBaseDamage` — 양수 BaseDmg 주입
- 에너미: `Data.EnemyAttackDamage` — 양수 AttackDmg 주입
- ExecCalc 출력 시 `-FinalDamage`로 HP 감산

**결과**: `GetSetByCallerMagnitude` 호출 태그가 Source 유형을 의미상으로 명시. ExecCalc 내 이중 부정 제거. 6개 파일 일괄 교체(BaseProjectile, BaseSummonObject, 에너미 4종).

**포트폴리오 포인트**: GAS 데이터 흐름 설계 — "입력은 양수, 출력에서 방향 결정" 원칙 / SetByCaller 태그 네이밍이 의미 전달하도록 분리하는 인터페이스 설계 사고.

**관련 파일**: `RSGameplayTags.h/.cpp`, `BaseProjectile.cpp`, `BaseSummonObject.cpp`, `MeleeEnemy.cpp`, `EliteEnemy.cpp`, `BossEnemy.cpp`, `EnemyProjectile.cpp`

---

## [2026-04-13] PATTERN — SUMMON 타입 자동발사에서 LocalInputConfirm 자동 호출

**상황**: M-3 자동발사 전환 시 SUMMON 타입 무기(召喚物을 일정 위치에 배치하는 GA)가 `WaitForPlayerConfirm` Task에서 무한 대기하는 문제. 수동 발사 모드에서는 플레이어 클릭이 `LocalInputConfirm`을 트리거했으나, 자동발사 모드에서는 클릭 입력이 없다.

**문제·과제**: `FireSlot` 호출 후 GA가 `WaitForPlayerConfirm` 상태에 멈춰 있으면 타이머 다음 틱에 동일 슬롯의 새 GA를 활성화하지 못하고 자동발사 루프가 정지.

**검토한 선택지**:
- A) SUMMON GA 내부에 타임아웃 추가 — GA 변경 필요, 다른 활성화 경로(수동 미래 지원)에 영향
- B) `StartAutoFire` 타이머 내에서 `FireSlot` 직후 `ASC->LocalInputConfirm()` 자동 호출 — 호출 측에서 처리, GA 무수정

**결정**: B 선택. `TriggerAbilityFromGameplayEvent`가 동기 처리되므로, `FireSlot` 반환 시점에 GA는 이미 `WaitForPlayerConfirm` 상태. 그 직후 `LocalInputConfirm()` 호출이 정확히 해당 대기를 해제.

**결과**: SUMMON 타입도 자동발사 루프 정상 동작. GA 코드 무수정.

**포트폴리오 포인트**: GAS `LocalInputConfirm`의 동작 시점 이해 / 자동화 컨텍스트에서 GA 내부를 변경하지 않고 호출 측에서 흐름을 제어하는 설계 판단.

**관련 파일**: `Source/RoastStaffGAS/Private/Subsystems/EquipmentSubsystem.cpp` (StartAutoFire 타이머 람다)

---

## [2026-04-13] ARCH — DT_CharacterSkill 스키마: 중첩 배열 vs 분리 테이블

**상황**: 캐릭터 고유 스킬의 레벨별 수치(Lv1/2/3 — DamageMultiplier, EffectRadius, Duration, FXClass)를 DataTable에 담는 방식 결정. 스킬당 3레벨, 캐릭터당 2슬롯의 소규모 데이터.

**문제·과제**: 두 가지 설계 선택지 존재.
- A안: `FCharacterSkillStaticData` 내 `TArray<FCharacterSkillLevelData>` 중첩 — 1테이블, 에디터 인라인 편집 가능. 단, CSV 임포트 불가(UE CSV는 중첩 배열 미지원).
- B안: `DT_CharacterSkillLevel` 별도 분리 — CSV 완전 지원. 단, 테이블 2개 + GDS 조회 시 JOIN 필요, 캐릭터 스킬 추가마다 두 테이블 모두 관리해야 하는 운영 부담 증가.

**검토한 선택지**: A안 vs B안. 핵심 기준은 데이터 규모, 편집 빈도, 조회 복잡도.
- 스킬 수: 캐릭터 수 × 2 = 최대 30개 내외 → 대규모 CSV 배치 임포트 불필요
- Lv1~3 데이터는 스킬 설계 시 한 번에 확정되고 이후 거의 수정 없음 → 빈번한 CSV 업데이트 불필요
- B안의 JOIN 조회는 `GetCharacterSkillExecData` 로직을 복잡하게 만들고 DT 행 수 3배 증가

**결정**: **A안 채택** — `TArray<FCharacterSkillLevelData>` 중첩 유지. DT_CharacterSkill은 에디터 직접 편집으로 관리, CSV 임포트 대상에서 제외.

**결과**: GDS 조회 `GetCharacterSkillExecData(CharID, Slot, Level)` 단일 함수로 Level 클램프+LevelData 인덱싱까지 처리. 테이블 1개, 조회 경로 단순.

**포트폴리오 포인트**: DataTable 아키텍처 설계 — "데이터 규모·편집 패턴·조회 복잡도" 세 기준으로 CSV 파이프라인 vs 에디터 직접 편집을 선택하는 판단 근거. UE DataTable의 CSV 제약(중첩 배열 미지원)을 인지하고 시스템별 적합한 편집 워크플로를 구분한 사례.

**관련 파일**: `DataTableStructs.h` (FCharacterSkillStaticData), `GameDataSubsystem.cpp` (GetCharacterSkillExecData), `ExternalSource/DT_Character_Skill_Static_Data.csv`

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

## [2026-04-22] BUG — 스테이지 클리어 후 로비 복귀 시 쿨타임 타이머 dangling 크래시

**상황**: 스킬 사용 중 스테이지를 클리어하면 로비로 레벨 전환되면서 에디터가 크래시. 스택 트레이스 최심단은 `USkillManagerSubsystem::StartCooldown` 내부 람다 → `OnSkillSlotUpdatedDel.Broadcast` → `ProcessMulticastDelegate` → `ConstructItems`(메모리 손상).

**문제·과제**: `StartCooldown`의 쿨타임 만료 타이머 람다가 raw `this`를 캡처. 레벨 전환 시 `USkillManagerSubsystem`(WorldSubsystem)은 소멸 경로에 진입하는데, `Deinitialize()` 오버라이드가 없어 타이머가 정리되지 않음. 타이머가 만료되면 소멸된 서브시스템의 `OnSkillSlotUpdatedDel`에 접근 → 델리게이트 내부 배열 오염 → `ConstructItems` 크래시.

**검토한 선택지**:
- A) `Deinitialize()`만 추가해 타이머를 강제 정리 — 타이머가 Deinitialize 이전에 만료될 경우 여전히 raw `this` 위험 잔존
- B) 람다만 `TWeakObjectPtr`로 교체 — 리스너(PC)가 소멸된 경우 델리게이트 오염 경로 차단 불완전
- C) 두 가지 모두 적용 — `Deinitialize()`에서 타이머 전체 Clear + 델리게이트 Clear, 람다는 `TWeakObjectPtr<USkillManagerSubsystem>` + `IsValid()` 가드

**결정**: C안 채택. `Deinitialize()`는 월드 해체 시 타이머와 델리게이트를 일괄 정리하는 방어선, `TWeakObjectPtr` 가드는 그 사이 타이머가 만료될 경우를 대비하는 이중 안전망. `PullVortexActor`가 이미 `TWeakObjectPtr` 패턴으로 동일 문제를 방지하고 있었음 — 서브시스템에도 동일 원칙 확산 적용.

**결과**: 레벨 전환 시 `Deinitialize()`에서 6개 슬롯 `CooldownTimer` 전부 `ClearTimer` → `OnSkillSlotUpdatedDel.Clear()` → `ASC = nullptr` → `bIsInitialized = false` 순으로 정리. 이후 람다가 실행되더라도 `WeakThis.IsValid()` false 분기에서 즉시 return.

**포트폴리오 포인트**: UE WorldSubsystem 생명주기와 TimerManager 정리 타이밍 이해 — `Deinitialize()` 미구현 시 레벨 전환 중 타이머 dangling이 발생하는 구조적 원인 분석. `TWeakObjectPtr` 이중 안전망 패턴을 프로젝트 내 일관 적용(PullVortexActor 선례 → SkillManagerSubsystem 확산).

**관련 파일**: `Source/RoastStaffGAS/Public/Subsystems/SkillManagerSubsystem.h`, `Source/RoastStaffGAS/Private/Subsystems/SkillManagerSubsystem.cpp`

---

<!-- 새 항목은 가장 최근 날짜가 위로 오도록 추가 -->