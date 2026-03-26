# PLAN_SlotUI_v1.0

> 작성일: 2026-03-16
> 관련 기획서: PLAN_SPRINT1_v1.2.md, UI관리 시스템 기획 v1.0.md
> 스프린트: SPRINT 1

---

## 구현 목표

슬롯 UI 위젯을 구현한다. 각 슬롯은 무기명, 쿨타임 오버레이(잔여 초), 모드 상태(자동/액티브 테두리)를 표시한다.
기존 K_ 접두사 참고용 클래스(UMS/BaseWidget/HUDWidget)를 RS 접두사 클래스로 재제작하고, K_ 파일을 삭제한다.
데이터 소스는 `EquipmentSubsystem`이며, 슬롯 상태 변경 시 `EquipmentComponent`가 델리게이트를 수신하여 UI를 갱신한다.

---

## 영향 범위

### 신규 생성 파일

| 파일 | 역할 | 비고 |
|------|------|------|
| `Public/UI/RSBaseWidget.h` | 모든 위젯의 베이스 클래스 | K_BaseWidget 대체 |
| `Private/UI/RSBaseWidget.cpp` | RSBaseWidget 구현 | K_BaseWidget 대체 |
| `Public/Subsystems/RSUIManagerSubsystem.h` | UI 생명주기 중앙 관리 서브시스템 | K_UIManagerSubsystem 대체 |
| `Private/Subsystems/RSUIManagerSubsystem.cpp` | RSUIManagerSubsystem 구현 | K_UIManagerSubsystem 대체 |
| `Public/UI/RSHUDWidget.h` | 인게임 HUD 위젯 | K_HUDWidget 대체 |
| `Private/UI/RSHUDWidget.cpp` | RSHUDWidget 구현 | K_HUDWidget 대체 |
| `Public/UI/RSSlotContainerWidget.h` | 슬롯 3개를 담는 컨테이너 | 신규 |
| `Private/UI/RSSlotContainerWidget.cpp` | RSSlotContainerWidget 구현 | 신규 |
| `Public/UI/RSSlotWidget.h` | 개별 슬롯 1개 위젯 | 신규 |
| `Private/UI/RSSlotWidget.cpp` | RSSlotWidget 구현 | 신규 |

### 수정 파일

| 파일 | 변경 내용 |
|------|-----------|
| `Public/Data/DataTableStructs.h` | `FWeaponStaticData::WeaponName` Category `"Weapon\|Future"` → `"Weapon"` 으로 변경 |
| `Public/Component/EquipmentComponent.h` | `URSHUDWidget*` 캐시 멤버 추가, `RSSlotContainerWidget` 포워드 선언 추가 |
| `Private/Component/EquipmentComponent.cpp` | `RefreshSlotUI()` 실제 구현, BeginPlay에서 HUD 위젯 캐시 획득 로직 추가 |

### 삭제 파일

| 파일 |
|------|
| `Public/UI/K_BaseWidget.h` |
| `Private/UI/K_BaseWidget.cpp` |
| `Public/Subsystems/K_UIManagerSubsystem.h` |
| `Private/Subsystems/K_UIManagerSubsystem.cpp` |
| `Public/UI/K_HUDWidget.h` |
| `Private/UI/K_HUDWidget.cpp` |

### DataTable 수정 여부
- `DataTableStructs.h` 코드 수정 후 에디터에서 CSV WeaponName 컬럼 값 입력 필요 (에디터 작업)

### Gameplay Tag 추가 여부
없음.

---

## K_ → RS 클래스 변경 사항 명세

### RSBaseWidget (K_BaseWidget 대비 변경점)

| 항목 | K_ (기존) | RS (변경) |
|------|-----------|-----------|
| `bIsModal` 프로퍼티 | 없음 | `UPROPERTY(EditDefaultsOnly)` 추가 |
| `CloseUI()` Visibility | `ESlateVisibility::Hidden` | `ESlateVisibility::Collapsed` (기획서 명세) |
| 델리게이트 타입 | `UK_BaseWidget*` | `URSBaseWidget*` |

### RSUIManagerSubsystem (K_UIManagerSubsystem 대비 변경점)

| 항목 | K_ (기존) | RS (변경) |
|------|-----------|-----------|
| 모든 위젯 참조 타입 | `UK_BaseWidget*` | `URSBaseWidget*` |
| `NotifyInputModeChange()` 입력 모드 | `bIsModal` 조건 없이 POPUP 존재 시 `FInputModeUIOnly` | bIsModal=true인 POPUP 존재 시 `FInputModeGameAndUI` (기획서 명세) |

### RSHUDWidget (K_HUDWidget 대비 변경점)

| 항목 | K_ (기존) | RS (변경) |
|------|-----------|-----------|
| `BindToASC()` | 미구현 빈 함수 | 제거 |
| `WBP_SlotContainer` | 없음 | `UPROPERTY(meta=(BindWidget))` 추가 |
| `NativeConstruct()` | 없음 | 추가 (SlotContainer ensureMsgf 유효성 체크) |

---

## 함수 호출 흐름

### Q1: 무기가 장착될 때 슬롯 UI는 어떻게 갱신되는가?

```
EquipmentSubsystem::CommitSlot()
  → OnSlotUpdatedDel.Broadcast(SlotIndex)
    → EquipmentComponent::OnSlotUpdated(SlotIndex)
      → RefreshSlotUI(SlotIndex)
        → EquipmentSubsystem::GetSlotData(SlotIndex)
        → CachedHUDWidget->GetSlotContainerWidget()
        → RSSlotContainerWidget::GetSlotWidget(SlotIndex)
        → RSSlotWidget::UpdateSlot(const FWeaponSlotInstanceData*)
```

### Q2: 쿨타임 잔여 시간은 어떻게 매 프레임 갱신되는가?

기획서 명세: "쿨타임 진행 중 — 매 프레임 잔여 초 텍스트 갱신"

```
RSSlotWidget::NativeTick(DeltaTime)
  └─ bIsCooldownActive == true 일 때만 실행
      → EquipmentSubsystem::GetSlotData(SlotIndex)
      → CooldownRemaining 읽어 텍스트 갱신
      → CooldownRemaining < 1.0f → "0" 표시
      → CooldownRemaining <= 0.0f → 오버레이 숨김, bIsCooldownActive = false
```

### Q3: 액티브 모드 전환 시 테두리 강조는 어떻게 갱신되는가?

```
EquipmentSubsystem::SetSlotActive() / ClearActiveSlot()
  → OnSlotUpdatedDel.Broadcast(SlotIndex)  ← 이미 구현됨 (EquipmentSubsystem.cpp 확인)
    → EquipmentComponent::OnSlotUpdated(SlotIndex)
      → RefreshSlotUI(SlotIndex)
        → RSSlotWidget::UpdateSlot()
          → bIsActive == true → 강조 테두리 ON
          → bIsActive == false → 강조 테두리 OFF
```

---

## 클래스 설계 명세

### URSBaseWidget (신규)

```
상속: UUserWidget

-- UPROPERTY --
EUILayer UILayer = PERSISTENT        // PERSISTENT 또는 POPUP
int32 ZOrder = 0
bool bIsModal = false                // true이면 GameAndUI 입력 모드로 전환
bool bIsOpen = false                 // 열림/닫힘 상태 (protected)
FOnCloseUIRequested OnCloseUIRequested  // BlueprintAssignable 델리게이트

-- 공개 인터페이스 --
virtual void OpenUI()                // bIsOpen=true, Visibility=Visible
virtual void CloseUI()               // bIsOpen=false, Visibility=Collapsed
virtual void RefreshUI()
virtual void OnFocusGained()
virtual void OnFocusLost()
FORCEINLINE bool IsOpen() const
```

### URSUIManagerSubsystem (신규)

K_ 구현과 구조 동일. 아래 항목만 변경:
- 모든 `UK_BaseWidget` → `URSBaseWidget`
- `NotifyInputModeChange()`:
  - POPUP 스택에 `bIsModal=true` 위젯이 하나라도 있으면 → `FInputModeGameAndUI`
  - 없으면 → `FInputModeGameOnly`

### URSSlotContainerWidget (신규)

```
상속: URSBaseWidget

-- BindWidget (BP에서 연결) --
URSSlotWidget* SlotWidget_0
URSSlotWidget* SlotWidget_1
URSSlotWidget* SlotWidget_2

-- 공개 인터페이스 --
URSSlotWidget* GetSlotWidget(int32 SlotIndex)
```

### URSSlotWidget (신규)

```
상속: URSBaseWidget

-- BindWidget (BP에서 연결) --
UTextBlock* Text_WeaponName           // 무기명 또는 "EMPTY"
UImage* Img_CooldownOverlay           // 쿨타임 오버레이
UTextBlock* Text_CooldownRemaining    // 잔여 초 텍스트
UImage* Img_ActiveBorder              // 액티브 모드 테두리 강조

-- 공개 인터페이스 --
void InitSlot(int32 InSlotIndex)
void UpdateSlot(const FWeaponSlotInstanceData* SlotData)

-- 내부 상태 --
int32 SlotIndex = -1
bool bIsCooldownActive = false        // Tick 활성화 조건

-- NativeTick --
쿨타임 진행 중일 때만 EquipmentSubsystem에서 CooldownRemaining 읽어 텍스트 갱신
bIsCooldownActive=false 시 SetIsTickable(false) 호출
```

### URSHUDWidget (신규)

```
상속: URSBaseWidget (UILayer = PERSISTENT)

-- BindWidget (BP에서 연결) --
URSSlotContainerWidget* WBP_SlotContainer

-- NativeConstruct --
ensureMsgf(WBP_SlotContainer, TEXT("[HUD] SlotContainerWidget BindWidget 누락"))
```

### UEquipmentComponent (수정)

```
추가 멤버:
URSHUDWidget* CachedHUDWidget = nullptr   // BeginPlay에서 UMS 통해 획득

BeginPlay 추가:
→ UMS->GetOrCreateWidget<URSHUDWidget>(...) 으로 캐싱

RefreshSlotUI(int32 SlotIndex) 구현:
1. CachedHUDWidget 유효성 확인 (KHS_WARN + return)
2. SlotIndex 범위 확인 (KHS_WARN + return)
3. WBP_SlotContainer->GetSlotWidget(SlotIndex) 획득
4. EquipmentSubsystem::GetSlotData(SlotIndex) 호출
5. SlotWidget->UpdateSlot(SlotData) 호출
```

---

## 슬롯 상태별 시각 표현 규칙

| 상태 | 발생 조건 | WeaponName | Overlay | ActiveBorder |
|------|-----------|------------|---------|--------------|
| 빈 슬롯 | `IsEmpty()` == true | "EMPTY" | 숨김 | 숨김 |
| 자동공격 대기 | 비액티브, 쿨타임 0 | WeaponName | 숨김 | 숨김 |
| 자동공격 쿨타임 중 | 비액티브, 쿨타임 > 0 | WeaponName | 표시 + 잔여 초 | 숨김 |
| 액티브 대기 | 액티브, 쿨타임 0 | WeaponName | 숨김 | 표시 |
| 액티브 쿨타임 중 | 액티브, 쿨타임 > 0 | WeaponName | 표시 + 잔여 초 | 표시 |

---

## 예외처리 목록

| 상황 | 처리 |
|------|------|
| HUDWidget 미초기화 상태에서 RefreshSlotUI 호출 | `KHS_WARN` 로그 출력 후 return |
| SlotIndex 범위 초과 (0~2 외) | `KHS_WARN` 로그 출력 후 return |
| GetSlotData() nullptr 반환 (빈 슬롯) | 빈 슬롯 상태로 위젯 갱신 ("EMPTY", 오버레이 제거) |
| 빈 슬롯인데 쿨타임 오버레이 표시 중 | UpdateSlot 내에서 즉시 오버레이 제거 |
| BindWidget 연결 누락 | NativeConstruct에서 ensureMsgf 로 조기 감지 |
| CooldownRemaining < 1.0f | "0"으로 표시 (기획서 예외처리 규칙) |

---

## 기획서 미정의 항목

| 항목 | 현황 | 처리 방침 |
|------|------|----------|
| WeaponName 표시값 | `FWeaponStaticData::WeaponName` 코드에 이미 존재. CSV 값 미입력 | DataTableStructs.h Category 변경 후 에디터에서 CSV 데이터 입력 |
| RSHUDWidget을 EquipmentComponent에서 어떻게 획득하는가 | UMS 통해 캐시된 인스턴스 획득. HUD는 PERSISTENT이므로 레벨 시작 시 GameMode가 열어놓음 | BeginPlay에서 UMS::GetOrCreateWidget<URSHUDWidget> 호출 |

---

## [검토 결과]

- **기획서 일관성**
  - SPRINT1_v1.2 UI/UX 섹션 및 UI관리 시스템 기획 v1.0과 일치.
  - `RSSlotWidget` / `RSSlotContainerWidget`은 UMS를 통해 직접 열리지 않는 자식 위젯으로 처리 → 기획서 "NONE은 자식 위젯 전용" 정책과 부합.
  - `NotifyInputModeChange()` 기획서 명세(`GameAndUI`) 반영.
  - `CloseUI()` Visibility `Collapsed` 반영.
- **누락된 예외처리**: 빈 슬롯에서 쿨타임 오버레이 잔존 시 즉시 제거 규칙 반영 완료.
- **기획서 정정 필요 사항**
  - `Weapon_Static_Data` 스키마에 `WeaponName` 컬럼 추가 명시 필요 (코드에는 이미 존재, 기획서 테이블 항목 추가)