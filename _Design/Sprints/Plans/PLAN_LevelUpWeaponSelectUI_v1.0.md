# PLAN_LevelUpWeaponSelectUI_v1.0

> 작성일: 2026-03-26
> 관련 기획서: 레벨업 시스템 기획 v1.2, 무기 시스템 기획 v1.2, UI관리 시스템 기획 v1.0
> 스프린트: SPRINT 1 — Task A

---

## 구현 목표

레벨업 발생 시 `LevelUpSubsystem.SelectWeaponCandidates()`이 후보 무기 3종의 카드 표시 데이터를 조립하고, `UIManagerSubsystem.OpenUI()`를 통해 `LevelUpWeaponSelectWidget` 팝업을 열어 플레이어가 선택한 무기를 `EquipmentSubsystem.EquipWeapon()`으로 장착한 뒤 `WeaponSlotContainerWidget`이 자동 갱신되는 전체 흐름을 구현한다.

---

## 영향 범위

### 수정 파일
| 파일 | 변경 내용 |
|------|-----------|
| `EnumTypes.h` | `EWeaponCardState` enum 추가 |
| `RuntimeDataStructs.h` | `FWeaponCardDisplayData` struct 추가 |
| `GameDataSubsystem.h` | `GetWeaponData()` private → public 승격 |
| `LevelUpSubsystem.h/.cpp` | `SelectWeaponCandidates()` 완성, `LevelUpUIClass` 필드 추가, 임시 자동장착 제거, `OnWeaponCandidatesReadyDel` 제거 |
| `WeaponSlotContainerWidget.h/.cpp` | `EquipmentSubsystem.OnSlotUpdatedDel` 구독 + `OnSlotUpdated()` 핸들러 추가 |

### 신규 파일
| 파일 | 내용 |
|------|------|
| `UI/LevelUpWeaponSelectWidget.h/.cpp` | 레벨업 무기 선택 팝업 위젯 |

### 삭제 파일
| 파일 | 사유 |
|------|------|
| `LevelUpComponent.h/.cpp` | 내용 없는 빈 껍데기. 역할 없음 |

### 변경 없음
| 파일 | 사유 |
|------|------|
| `RSHUDWidget.h/.cpp` | UIManagerSubsystem이 팝업 직접 관리 — HUD 수정 불필요 |
| `WeaponSlotWidget.h/.cpp` | 기존 `UpdateSlot()` 재사용, 변경 불필요 |
| `EquipmentSubsystem.h/.cpp` | `EquipWeapon()`, `GetSlotData()`, `OnSlotUpdatedDel` 기존 인터페이스로 충분 |
| `EquipmentComponent.h/.cpp` | Task A 범위 외. 빈 껍데기 유지 |

---

## 데이터 구조 정의

### EnumTypes.h — `EWeaponCardState` 추가
```cpp
UENUM(BlueprintType)
enum class EWeaponCardState : uint8
{
    New      UMETA(DisplayName = "NEW"),       // 미보유 신규 무기
    Lv1ToLv2 UMETA(DisplayName = "Lv1→Lv2"),  // 동일 BaseType Lv1 보유 중
    Lv2ToLv3 UMETA(DisplayName = "Lv2→Lv3"),  // 동일 BaseType Lv2 보유 중
    Lv3Max   UMETA(DisplayName = "Lv3(MAX)"),  // 동일 BaseType Lv3 보유 (최대)
};
```

### RuntimeDataStructs.h — `FWeaponCardDisplayData` 추가
```cpp
USTRUCT(BlueprintType)
struct FWeaponCardDisplayData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly) FName WeaponID;
    UPROPERTY(BlueprintReadOnly) FName WeaponName;
    UPROPERTY(BlueprintReadOnly) FName Description;
    UPROPERTY(BlueprintReadOnly) EWeaponCardState CardState = EWeaponCardState::New;
    UPROPERTY(BlueprintReadOnly) bool bCanEvolve = false; // DT_Combination 미구현 — 항상 false (스텁)
};
```

---

## 함수 호출 흐름

### Q1. 레벨업 발생 시 UI까지 어떻게 흘러가는가?

```
LevelUpSubsystem::CheckLevelUp()
  └→ ApplyLevelUp()           ← 어트리뷰트 갱신 (Level+1, EXP 이월)
  └→ SelectWeaponCandidates() ← 무기 후보 선정 + UI 오픈
```

### Q2. `SelectWeaponCandidates()` 내부 흐름은?

```
① GDS->GetWeaponIDsByLevel(1) → WeaponPool
② Algo::RandomShuffle(WeaponPool)
③ PickCount = min(WeaponPool.Num(), 3)
   → 유효 무기 3종 미만 시: 가능한 수량만 진행. 0종이면 경고 후 리턴
④ For each CandidateID:
   a. GDS->GetWeaponData(CandidateID, WeaponData)  ← public으로 승격
   b. CardState 결정:
      For i in 0..2:
        SlotData = EquipSys->GetSlotData(i)
        if SlotData && !SlotData->IsEmpty():
          GDS->GetWeaponData(SlotData->SlotEquipData.WeaponID, EquippedData)
          if EquippedData.BaseType == WeaponData.BaseType:
            CardState = WeaponLevel 1→Lv1ToLv2 / 2→Lv2ToLv3 / 3→Lv3Max
      BaseType 일치 슬롯 없으면 → New
   c. bCanEvolve = false (스텁)
   d. Candidates.Add(FWeaponCardDisplayData{...})
⑤ UMS = GetGameInstance()->GetSubsystem<UUIManagerSubsystem>()
⑥ Widget = UMS->OpenUI<ULevelUpWeaponSelectWidget>(LevelUpUIClass)
⑦ Widget->SetCandidates(Candidates)
⑧ GetWorld()->GetWorldSettings()->SetTimeDilation(0.f)  ← 게임 일시정지
   ※ 추후 StageSystem 위임으로 교체 예정
⑨ Widget->OnWeaponSelectCompletedDel.AddDynamic(this, &ULevelUpSubsystem::OnWeaponSelectCompleted)
```

### Q3. `LevelUpWeaponSelectWidget` 인터페이스는?

```cpp
// 공개 인터페이스
void SetCandidates(const TArray<FWeaponCardDisplayData>& InCandidates);

// 내부 이벤트 (BP 바인딩)
void OnCardSelected(int32 CardIndex);   // 카드 [선택] 버튼 클릭
void OnConfirmClicked();               // [확인] 버튼 클릭
void OnCloseButtonClicked();           // [X] 버튼 클릭

// 델리게이트 (LevelUpSubsystem이 구독)
UPROPERTY(BlueprintAssignable)
FOnWeaponSelectCompleted OnWeaponSelectCompletedDel;
```

RSBaseWidget 상속 → UILayer = POPUP, bIsModal = true

### Q4. 플레이어가 무기를 선택하고 [확인]을 누르면?

```
OnConfirmClicked():
  ① if (SelectedCardIndex < 0) return  ← 선택 없이 확인 불가 (버튼 비활성 상태 보호)
  ② EquipSys->EquipWeapon(Candidates[SelectedCardIndex].WeaponID)
  ③ OnCloseRequested()  ← URSBaseWidget → UIManagerSubsystem.CloseUI() 자동 호출
  ④ OnWeaponSelectCompletedDel.Broadcast()

OnCloseButtonClicked() [X] 버튼:
  ① OnCloseRequested()
  ② OnWeaponSelectCompletedDel.Broadcast()
```

### Q5. UIManagerSubsystem이 CloseUI를 받으면?

```
CloseUIInternal(Widget):
  ① PopupUIStack에서 제거
  ② Widget->CloseUI()   → Visibility = Collapsed
  ③ 새 Top에 OnFocusGained()
  ④ NotifyInputModeChange()
     → 스택 비면 GameOnly 모드 + 커서 숨김
```

### Q6. LevelUpSubsystem은 widget 닫힘을 어떻게 감지하는가?

```
OnWeaponSelectCompleted():
  ① GetWorld()->GetWorldSettings()->SetTimeDilation(1.f)  ← 게임 재개
  ② Widget->OnWeaponSelectCompletedDel.RemoveDynamic(...)  ← 구독 해제 (재사용 대비)
```

### Q7. 무기 장착 후 HUD 슬롯은 어떻게 갱신되는가?

```
EquipmentSubsystem::CommitSlot()
  └→ OnSlotUpdatedDel.Broadcast(SlotIndex)
       └→ WeaponSlotContainerWidget::OnSlotUpdated(SlotIndex)  ← 신규 구독
            └→ GetSlotWidget(SlotIndex)->UpdateSlot(EquipSys->GetSlotData(SlotIndex))
```

WeaponSlotContainerWidget::NativeConstruct()에 구독 코드 추가:
```cpp
EquipSys->OnSlotUpdatedDel.AddDynamic(this, &UWeaponSlotContainerWidget::OnSlotUpdated);
```

---

## 예외처리 목록

| # | 상황 | 처리 |
|---|------|------|
| 1 | `LevelUpUIClass` 미설정 | `ensureMsgf` 실패 → 경고 로그 + 리턴 |
| 2 | 무기 풀 0종 | 경고 로그 + check(false) |
| 3 | 무기 풀 1~2종 | 가능한 수량만 UI 표시 |
| 4 | `GetWeaponData` 조회 실패 | 경고 로그 + 해당 카드 건너뜀 |
| 5 | 후보 0종 (모두 조회 실패) | 경고 로그 + Widget 닫기 + 시간 재개 |
| 6 | 이미 레벨업 UI 열린 상태 | bIsLevelingUp 가드 |
| 7 | [확인] 클릭 시 선택 없음 | 버튼 비활성 유지 (BP 처리) + C++ 방어 코드 |
| 8 | EquipWeapon 실패 | EquipmentSubsystem 기존 예외처리 위임 |
| 9 | OnWeaponSelectCompleted 구독 해제 누락 | RemoveDynamic 명시적 호출 |

---

## Task A 범위 외 항목 (기획서 미구현)

| 항목 | 상태 |
|------|------|
| 진화 가능 여부 판단 (DT_Combination) | `bCanEvolve = false` 스텁. 별도 Task로 분리 필요 |
| 무기 아이콘 표시 (DT_WeaponResources) | DataTableStructs에 미존재. 스텁 처리 (icon null) |
| 무기 교체 UI 전환 (슬롯 가득 찼을 때) | EquipmentSubsystem 교체 신호 수신 후 WEAPON_REPLACE 팝업 — 별도 Task |
| 연속 레벨업 처리 (잉여 EXP로 즉시 재레벨업) | while 루프 내 다중 호출 문제. Task B에서 큐 방식으로 개선 필요 |
| 게임 일시정지 StageSystem 위임 | 현재 직접 SetTimeDilation 호출. 추후 위임 |
| 카드 등장 애니메이션 / SFX | BP 레벨 구현. C++ 범위 외 |

---

## [검토 결과]

- **기획서 일관성**: 레벨업 시스템 기획 v1.2 및 UI관리 시스템 기획 v1.0과 일치. UIManagerSubsystem 경유 원칙 준수.
- **누락된 예외처리**: 연속 레벨업 시 while 루프 내 다중 SelectWeaponCandidates() 호출로 첫 번째 UI가 닫히기 전 두 번째 UI 오픈 시도 가능 — Task B에서 큐 구조로 개선 필요. Task A는 단일 레벨업 흐름만 대상.
- **기획서 정정 필요 사항**: 없음
- **Gemini 리뷰**: 사용자 요청으로 생략. 계획서 직접 승인.
