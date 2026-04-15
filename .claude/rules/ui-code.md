# UI/UMG 코드 규칙
> 적용 경로: Source/**/UI/**
> CODE 단계에서 UI 파일 작업 시 반드시 준수

## 필수 규칙

### 델리게이트 바인딩 위치 (자기주도 발견 취약점)
- `AddDynamic` → **NativeOnInitialized** 에만 — NativeConstruct 금지
  - NativeConstruct는 캐시/풀링 재사용 시 중복 바인딩 버그 발생
  - 정리(RemoveDynamic)는 NativeDestruct 에서
- `Super::NativeOnInitialized()` 호출 후 바인딩 수행

### Input Mode + Mouse Cursor (삭제 편향 취약점)
- `SetInputMode(...)` + `SetShowMouseCursor(...)` 항상 쌍으로 작성
- 한 쪽만 변경 시 다른 쪽 영향 반드시 검토

### UI/게임 상태 분리
- Widget은 게임 상태를 직접 소유하거나 수정 금지
  - 읽기: Subsystem / Delegate / GAS AttributeSet에서 데이터 수신
  - 쓰기: Event / Command 패턴으로 게임 시스템에 위임
- Widget이 직접 ASC에 GE Apply → 금지

### 위젯 생명주기
- `IsInViewport()` 확인 없이 `RemoveFromParent()` 호출 금지
- 레벨 전환 후 캐시된 Widget → dangling 상태 가능성 확인
  - `IsOpen() && !IsInViewport()` 패턴으로 dangling 감지

### Visibility
- `SetVisibility(ESlateVisibility::Collapsed)` — 레이아웃에서 제거
- `SetVisibility(ESlateVisibility::Hidden)` — 공간 유지 (의도적 사용 시만)
- 의도 없이 Hidden 사용 금지

### 텍스트
- 표시 텍스트 → `FText` 사용 — `FString` 금지
- SetText(FText::FromString(...)) 도 금지 — FText 리터럴 또는 LOCTEXT 사용

### 위젯 풀링/캐싱
- Pool에서 재사용되는 위젯 → Release 시 초기 상태로 완전 리셋
  - 텍스트, 이미지, Visibility, 애니메이션 상태 전부
- `OnPoolDeactivate()` 구현 누락 금지 (poolable actor 패턴 준용)

## 코드 작성 전 체크리스트
- [ ] AddDynamic이 NativeOnInitialized에 있는가 (NativeConstruct 아님)
- [ ] SetInputMode 변경 시 SetShowMouseCursor도 같이 처리했는가
- [ ] Widget이 게임 상태를 직접 수정하는 코드가 없는가
- [ ] 레벨 전환 후 dangling 가능성 있는 캐시가 없는가
- [ ] 풀링 재사용 위젯의 상태 리셋이 완전한가
