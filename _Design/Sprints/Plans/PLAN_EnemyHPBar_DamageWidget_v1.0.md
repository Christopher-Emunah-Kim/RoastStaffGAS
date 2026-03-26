# PLAN_EnemyHPBar_DamageWidget_v1.0

> 작성일: 2026-03-26
> 관련 기획서: `AI_에너미 시스템 기획 v1.1`, `UI관리 시스템 기획 v1.0`
> 스프린트: feature/enemy-hpbar

---

## 구현 목표

에너미 캐릭터에 체력바(`EnemyHPBarWidget`)와 데미지 플로팅 텍스트(`EnemyDamageWidget`)를 구현한다.

| 위젯 | 방식 | 관리 주체 |
|---|---|---|
| `UEnemyHPBarWidget` | WidgetComponent로 에너미에 부착 | `AEnemyBaseCharacter` 소유 |
| `UEnemyDamageWidget` | 뷰포트 3D→2D 프로젝션 플로팅 | `ARSPlayerController` 풀 소유 |

---

## 영향 범위

### 신규 생성 (4파일)

| 파일 | 역할 |
|---|---|
| `Public/UI/Enemy/EnemyHPBarWidget.h` | 체력바 위젯 |
| `Private/UI/Enemy/EnemyHPBarWidget.cpp` | |
| `Public/UI/Enemy/EnemyDamageWidget.h` | 데미지 플로팅 위젯 |
| `Private/UI/Enemy/EnemyDamageWidget.cpp` | |

### 수정 (2클래스)

| 파일 | 변경 내용 |
|---|---|
| `EnemyBaseCharacter.h/.cpp` | `UWidgetComponent` 추가, `SetupHPBar()`, HP변경 → `PC::SpawnDamageWidget()` 호출 |
| `RSPlayerController.h/.cpp` | DamageWidget Pool 소유, `SpawnDamageWidget(FVector, float)`, `ReturnDamageWidgetToPool(UEnemyDamageWidget*)` |

DataTable: 없음 / Gameplay Tag: 없음

---

## 함수 호출 흐름

### 1. 초기화 — WidgetComponent 생성 및 HPBar 바인딩

```
AEnemyBaseCharacter::AEnemyBaseCharacter()  [생성자]
  └─ HPBarWidgetComp = CreateDefaultSubobject<UWidgetComponent>("HPBarWidgetComp")
  └─ HPBarWidgetComp->SetupAttachment(RootComponent)
  └─ HPBarWidgetComp->SetWidgetSpace(EWidgetSpace::World)
  └─ HPBarWidgetComp->SetRelativeLocation(FVector(0, 0, HPBarZOffset))

AEnemyBaseCharacter::InitializeEnemy(EnemyID)
  └─ InitializeAbilitySystem()
  └─ ApplyStatData()
  └─ SetupHPBar()
       └─ HPBarWidgetComp->SetWidgetClass(HPBarWidgetClass)
       └─ UEnemyHPBarWidget* W = Cast<UEnemyHPBarWidget>(HPBarWidgetComp->GetWidget())
       └─ W->BindToASC(ASC)
  └─ SetupDamageDelegate()
       └─ ASC->GetGameplayAttributeValueChangeDelegate(CurrentHP)
               .AddUObject(this, &OnCurrentHPChangedForDamage)
```

```
UEnemyHPBarWidget::BindToASC(UAbilitySystemComponent* InASC)
  └─ !InASC → 경고 로그, 반환
  └─ CachedASC = InASC
  └─ TargetHealth     = ASC->GetNumericAttribute(CurrentHP)   ← 초기값 즉시 조회
  └─ CurrentMaxHealth = ASC->GetNumericAttribute(MaxHP)
  └─ GhostHealth      = TargetHealth
  └─ PBar_Health->SetPercent(TargetHealth / CurrentMaxHealth) ← 초기 렌더링
  └─ PBar_Ghost->SetPercent(TargetHealth / CurrentMaxHealth)
  └─ ASC->GetGameplayAttributeValueChangeDelegate(CurrentHP).AddUObject(this, &OnCurrentHPChanged)
  └─ ASC->GetGameplayAttributeValueChangeDelegate(MaxHP).AddUObject(this, &OnMaxHPChanged)
```

---

### 2. HP 감소 → HPBar 갱신

```
GE 적용 → BaseAttributeSet::PostGameplayEffectExecute → ASC AttributeChange Broadcast

UEnemyHPBarWidget::OnCurrentHPChanged(FOnAttributeChangeData)
  └─ FMath::IsNearlyEqual(TargetHealth, Data.NewValue, 0.01f) → 반환
  └─ TargetHealth = Data.NewValue
  └─ CurrentMaxHealth = ASC->GetNumericAttribute(MaxHP)  ← 최신값 재조회
  └─ PBar_Health->SetPercent(TargetHealth / CurrentMaxHealth)
  └─ GhostDelayTimer = GhostDelayTime
  └─ CheckLowHealthState()
       └─ CurrentMaxHealth <= 0 → 반환
       └─ bShouldBeLowHealth = (TargetHealth/MaxHP <= LowHealthThreshold) && (TargetHealth > 0)
       └─ bShouldBeLowHealth == bIsLowHealth → 반환  (중복 방지)
       └─ true:  Img_DangerGlow→SelfHitTestInvisible, Anim_LowHealth 재생(무한)
       └─ false: Img_DangerGlow→Hidden, Anim_LowHealth 정지

UEnemyHPBarWidget::OnMaxHPChanged(FOnAttributeChangeData)
  └─ CurrentMaxHealth = Data.NewValue

NativeTick → UpdateGhostBar(DeltaTime)
  └─ GhostDelayTimer > 0 → GhostDelayTimer -= DeltaTime, 반환
  └─ !IsNearlyEqual(GhostHealth, TargetHealth, 0.1f)
       └─ GhostHealth = FInterpTo(GhostHealth, TargetHealth, DeltaTime, InterpSpeed_Ghost)
       └─ PBar_Ghost->SetPercent(GhostHealth / CurrentMaxHealth)
```

---

### 3. HP 감소 → DamageWidget 스폰 (PC 경유)

```
AEnemyBaseCharacter::OnCurrentHPChangedForDamage(FOnAttributeChangeData)
  └─ Damage = Data.OldValue - Data.NewValue
  └─ Damage <= 0.f → 반환  (회복 미표시)
  └─ ARSPlayerController* PC = Cast<ARSPlayerController>(GetWorld()->GetFirstPlayerController())
  └─ !PC → 경고 로그 → 반환
  └─ FVector WorldPos = GetActorLocation() + FVector(0, 0, DamageWidgetZOffset)
  └─ PC->SpawnDamageWidget(WorldPos, Damage)

ARSPlayerController::SpawnDamageWidget(FVector WorldPos, float Damage)
  └─ !DamageWidgetClass → 경고 로그 → 반환
  └─ Pool에서 꺼내기 (있으면) 또는 CreateWidget<UEnemyDamageWidget>(this, DamageWidgetClass)
  └─ FVector2D ScreenPos
  └─ ProjectWorldLocationToScreen(WorldPos, ScreenPos, true) == false → 반환 (화면 밖)
  └─ Widget->SetPositionInViewport(ScreenPos, false)
  └─ !Widget->IsInViewport() → Widget->AddToViewport(200)
  └─ Widget->SetVisibility(ESlateVisibility::SelfHitTestInvisible)
  └─ Widget->PlayFloatAnimation(Damage)

UEnemyDamageWidget::PlayFloatAnimation(float Damage)
  └─ !Txt_Amount → 경고 로그
  └─ Txt_Amount->SetText(FText::AsNumber((int32)Damage))
  └─ BindToAnimationFinished(FadeAnimation, OnFadeFinished)
  └─ PlayAnimation(FadeAnimation)

UEnemyDamageWidget::OnFadeFinished()
  └─ UnbindAllFromAnimationFinished(FadeAnimation)
  └─ ARSPlayerController* PC = Cast<ARSPlayerController>(GetOwningPlayer())
  └─ PC 유효 → PC->ReturnDamageWidgetToPool(this)
  └─ PC 무효 → RemoveFromParent() 후 반환  (PC 소멸 예외)

ARSPlayerController::ReturnDamageWidgetToPool(UEnemyDamageWidget* Widget)
  └─ Widget->RemoveFromParent()
  └─ Widget->SetVisibility(ESlateVisibility::Collapsed)
  └─ DamageWidgetPool.Add(Widget)
```

---

### 4. 에너미 사망 — 정리

```
AEnemyBaseCharacter::HandleDeath()
  └─ Super::HandleDeath()
  └─ HPBarWidgetComp->SetVisibility(false)
  └─ ASC->GetGameplayAttributeValueChangeDelegate(CurrentHP).RemoveAll(this)
       ← DamageDelegate 명시 해제

UEnemyHPBarWidget::NativeDestruct()
  └─ CachedASC 유효 시:
       GetGameplayAttributeValueChangeDelegate(CurrentHP).RemoveAll(this)
       GetGameplayAttributeValueChangeDelegate(MaxHP).RemoveAll(this)
  └─ Super::NativeDestruct()
```

---

## UPROPERTY 목록

### EnemyHPBarWidget

| 프로퍼티 | 타입 | 메타 | 기본값 |
|---|---|---|---|
| `PBar_Health` | `UProgressBar*` | BindWidget | — |
| `PBar_Ghost` | `UProgressBar*` | BindWidget | — |
| `Img_DangerGlow` | `UImage*` | BindWidgetOptional | — |
| `Anim_LowHealth` | `UWidgetAnimation*` | Transient | — |
| `GhostDelayTime` | `float` | EditDefaultsOnly | 0.8f |
| `InterpSpeed_Ghost` | `float` | EditDefaultsOnly | 3.0f |
| `LowHealthThreshold` | `float` | EditDefaultsOnly | 0.3f |

### EnemyBaseCharacter (추가)

| 프로퍼티 | 타입 | 메타 | 기본값 |
|---|---|---|---|
| `HPBarWidgetComp` | `UWidgetComponent*` | VisibleAnywhere | — |
| `HPBarWidgetClass` | `TSubclassOf<UEnemyHPBarWidget>` | EditDefaultsOnly | — |
| `DamageWidgetClass` | `TSubclassOf<UEnemyDamageWidget>` | EditDefaultsOnly | — |
| `HPBarZOffset` | `float` | EditDefaultsOnly | 90.0f |
| `DamageWidgetZOffset` | `float` | EditDefaultsOnly | 120.0f |

### RSPlayerController (추가)

| 프로퍼티 | 타입 | 메타 |
|---|---|---|
| `DamageWidgetPool` | `TArray<TObjectPtr<UEnemyDamageWidget>>` | UPROPERTY() |
| `DamageWidgetClass` | `TSubclassOf<UEnemyDamageWidget>` | EditDefaultsOnly |

---

## 예외처리 목록

| 상황 | 처리 |
|---|---|
| `BindToASC` 시 ASC null | 경고 로그, 바인딩 스킵 |
| MaxHP = 0 (나눗셈) | Percent = 0.0f |
| OldHP ≈ NewHP (`IsNearlyEqual`) | 조기 반환 |
| HP 회복 (Damage ≤ 0) | DamageWidget 미표시 |
| `bIsLowHealth` 중복 토글 | 플래그 비교 후 변경 시에만 처리 |
| PC null 또는 캐스트 실패 | 경고 로그, 반환 |
| `DamageWidgetClass` null | 경고 로그, 반환 |
| 화면 밖 에너미 (`ProjectWorldToScreen` == false) | DamageWidget 미스폰 |
| `HandleDeath` 시 DamageDelegate 미해제 | `RemoveAll(this)` 명시 호출 |
| `OnFadeFinished` 시 PC 소멸 | `RemoveFromParent()`만 호출 후 반환 |

---

## [검토 결과]

### 기획서 일관성
- `AI_에너미 시스템 기획 v1.1` §UI/UX "모든 적 클래스는 공통으로 체력바 UI 컴포넌트를 보유" → 충족
- `UI관리 시스템 기획 v1.0` "모든 위젯은 UMS를 통해서만" → `EnemyDamageWidget`은 PERSISTENT/POPUP 레이어 대상 외의 **월드 이벤트 기반 일시적 UI 이펙트**로 분류. 기획서 예외 조항 추가로 해결.

### 기획서 정정 사항
- `UI관리 시스템 기획 v1.0` 위젯 목록 하단에 예외 조항 추가 → **완료**

### Gemini 리뷰 반영
- **반영**: WidgetComponent 부착 방식 명시, 초기값 즉시 렌더링, DamageWidget 좌표 기준점(ZOffset) 명시, `ProjectWorldToScreen` false 시 미스폰, `DamageWidgetClass`/`HPBarWidgetClass` UPROPERTY 명시, `OnFadeFinished` PC 소멸 예외 처리
- **미반영**: FDelegateHandle 핸들별 관리 → `RemoveAll(this)` 단일 구독자에서 충분, 오버엔지니어링
- **미반영**: NativeTick→Timer 최적화 → UMG 표준 패턴, 이번 스프린트 범위 외
- **미반영**: 다중 피격 클러터 방지 → 이번 스프린트 범위 외

### 설계 변경 (사용자 검토 반영)
- `PoolingSubsystem` Widget Pool 추가 → **철회**. DamageWidget은 `ARSPlayerController`가 직접 Pool 소유. UI 생성의 자연스러운 소유자(PC)에 위치시키고, PoolingSubsystem의 단일 책임(Actor Pool) 유지.