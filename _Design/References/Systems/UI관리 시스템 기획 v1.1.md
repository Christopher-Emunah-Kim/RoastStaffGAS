# [🟩 Solo Project] (12) UI 관리 시스템 기획 v1.1

No: 431
주제: C++, Self
난이도: ⭐⭐
최종수정: 2026년 3월 31일
작성일: 2026년 3월 2일 오후 5:31
Keyword: 기획

# 💡 INDEX

# UI 관리 시스템 기획서

## 💡 핵심 역할

> 
> 
> 
> 
> - UI 관리 시스템은 게임 내 모든 위젯의 생성, 표시, 전환, 제거를 단일 창구로 처리하는 중앙 집중식 관리자 시스템이다.
> - 어떤 게임 시스템도 위젯을 직접 생성하거나 뷰포트에 추가하지 않는다.
> - 반드시 UI 관리 시스템을 통해서만 위젯을 열고 닫는다.
> - 이하 문서에서 `UIManagerSubsystem`을 **UMS**로 약어 사용.
> 
> ### 핵심 책임
> 
> UMS는 네 가지 핵심 책임을 가진다.
> 
> 1. **위젯 생명주기 관리**
>     - 위젯의 생성, 캐싱, 뷰포트 추가/제거, 최종 소멸까지의 전체 생명주기를 관리
>     - 한 번 생성된 위젯은 캐시에 보관되어 반복 생성 비용을 줄임
> 2. **UI 레이어 관리**
>     - 모든 위젯은 PERSISTENT / PAGE / POPUP / SYSTEM 레이어 중 하나에 속하며, 레이어에 따라 ZOrder와 관리 방식이 결정
> 3. **Popup 스택 관리**
>     - 여러 팝업이 중첩될 때 포커스 순서를 보장하고, 최상위 팝업이 닫히면 그 아래 팝업에 포커스를 자동으로 전달
> 4. **입력 모드 자동 전환**
>     - 모달 팝업이 열리면 `GameAndUI` 입력 모드로 전환하고, 
>     모든 팝업이 닫히면 `GameOnly` 입력 모드로 복귀
> 

---

## 💡 레벨 구조 및 화면 전환 전체 흐름

> 
> 
> 
> 
> - 전체 레벨은 아래 4개 레벨로 구성된다.
> 
> | 레벨 | 역할 |
> | --- | --- |
> | MainMenu 레벨 | 타이틀 화면. 게임 시작/종료 진입점 |
> | Transition 레벨 | 레벨 간 이동 시 사전 로딩 처리. 로딩 화면 표시 |
> | OutGame 레벨 | 캐릭터 선택, 스테이지 선택 화면. 아웃게임 전체 |
> | InGame 레벨 | 인게임 전투, 레벨업 UI, 결과 화면, 리더보드 |
> 
> 전체 화면 전환 흐름은 다음과 같다.
> 
> ```cpp
> [MainMenu 레벨]
>   메인메뉴 PERSISTENT 위젯 표시
>   → 게임 시작 버튼 클릭
>   → GameInstance.OpenNextLevel(Lobby) 호출
>   → UIManager.ResetAllUIStates() 실행
>   → [Transition 레벨] 진입
>       로딩 화면 표시 (GameInstance 직접 관리)
>       OutGame 레벨 사전 로딩
>       로딩 완료 → FinishLoading()
>       → [OutGame 레벨] 진입
>           OutGame 루트 위젯 PERSISTENT 표시
>           → 캐릭터 선택 POPUP 오픈
>           → (선택 완료) 캐릭터 선택 POPUP 닫기
>           → 스테이지 선택 POPUP 오픈
>           → (선택 완료) GameInstance.OpenNextLevel(InGame) 호출
>           → [Transition 레벨] 진입
>               로딩 화면 표시
>               InGame 레벨 사전 로딩
>               → [InGame 레벨] 진입
>                   HUD PERSISTENT 표시
>                   → (전투 중) 레벨업 POPUP, 교체 POPUP 스택 관리
>                   → (스테이지 종료) 결과 화면 POPUP 표시
>                   → (결과 확인) 리더보드 POPUP 표시
>                   → 메인 메뉴로 버튼 → GameInstance.OpenNextLevel(MainMenu)
> ```
> 

---

## 💡 다른 시스템과의 관계

> **UMS는 위젯 자체의 데이터를 알지 못한다.** 
UMS는 위젯을 열고 닫는 것만 담당하며, 
위젯 내부에서 어떤 데이터를 표시하는지는 각 위젯이 직접 RDS, GDS에 조회하여 처리한다.
> 

| 연관 시스템 | 관계 방향 | 설명 |
| --- | --- | --- |
| GameInstance | GameInstance → UMS | 레벨 전환 시 `ResetAllUIStates()` 호출. 
로딩 화면은 GameInstance가 직접 관리. |
| 스테이지 시스템 | 스테이지 → UMS | 레벨업 이벤트, 스테이지 종료 시 해당 UI 오픈 요청. |
| 레벨업 시스템 | 레벨업 → UMS | 레벨업 UI 오픈/클로즈 요청. |
| RDS | UMS → RDS | 로비 화면에서 해금 캐릭터 목록, 스테이지 기록 조회. |
| 플레이어 조작 시스템 | UMS → 조작 | 팝업 열림/닫힘 시 입력 모드 변경을 조작 시스템에 통보. |

---

## 💡 UI 레이어 정의

> 
> 
> 
> 
> - 레이어는 4개로 구성된다. 모든 위젯은 반드시 하나의 레이어에 속해야 한다.
>
>     | 레이어 | ZOrder | 관리 방식 | 설명 |
>     | --- | --- | --- | --- |
>     | PERSISTENT | 100 | PersistentUIMap | HUD 등 항상 표시. 레벨 내 배경 레이어. |
>     | PAGE | 200 | UIHistory 스택 | 메인 콘텐츠. 동시에 1개. BackPage()로 이전 페이지 복귀. |
>     | POPUP | 300+ | PopupUIStack (스택당 +10) | 모달. 여러 개 중첩 가능. |
>     | SYSTEM | 500 | 단일 인스턴스 | 종료 확인/에러. 최상위. |
>
> - PERSISTENT는 씬 내에서 항상 표시되는 배경 레이어, 다른 레이어가 열려도 사라지지 않음.
> - PAGE는 동시에 1개만 표시되며, SwitchPageUI(EUIID)로 전환하고 UIHistory 스택에 이전 페이지를 기록한다.
> - POPUP은 PAGE 위에 쌓이며, 나중에 열린 것이 더 높은 ZOrder를 가져 항상 최상위에 표시.
> 

---

## 💡 기능 명세 및 상세 규칙

### 위젯 생성 및 캐싱 상세 규칙

> 
> 
> 
> 
> - UMS는 위젯을 처음 요청받는 시점에 생성하고 `CachedWidgets` 맵에 보관한다.
> - 이후 동일한 위젯 클래스가 요청되면 새로 생성하지 않고 캐시에서 반환한다.
> 
> - 위젯은 닫혀도 캐시에서 제거되지 않는다.
> - 단순히 `Visibility`를 `Collapsed`로 전환하여 화면에서 숨긴다.
> - 이 방식은 자주 열고 닫는 위젯(레벨업 UI, 일시정지 메뉴 등)의 반복 생성 비용을 제거하고, 
> 위젯의 내부 상태를 닫은 후에도 보존하는 이점을 제공한다.
> - 레벨 전환 시에는 `ResetAllUIStates()`를 호출하여 모든 캐시를 비우고 위젯을 소멸시킨다. 
> 이로써 이전 레벨의 위젯이 다음 레벨에 잔존하는 문제를 방지한다.
> 
> **예외처리:**
> 
> - `PlayerController`가 null인 상태에서 위젯 생성 요청
>     - 생성 중단, 경고 로그 출력, nullptr 반환
> - 이미 열려있는 위젯에 OpenUI 재요청
>     - `RefreshUI()`만 호출하고 즉시 반환. 중복 등록 방지
> - UILayer가 NONE으로 설정된 위젯에 OpenUI 요청
>     - 경고 로그 출력 후 nullptr 반환. NONE은 자식 위젯 전용 레이어이므로 직접 오픈 불가
> 

### 위젯 열기(OpenUI) 상세 규칙

> 
> 
> 
> 
> - `OpenUI`가 호출되면 UMS는 아래 순서로 처리한다.
>     1. 먼저 캐시에서 위젯을 가져오거나 새로 생성한다. 
>     2. 이어서 위젯의 `UILayer`를 확인하여 
>     PERSISTENT이면 `PersistentUIMap`에 등록하고 뷰포트에 추가한다. 
>     3. POPUP이면 
>         1. 현재 스택 최상위 위젯에 `OnFocusLost()`를 통보한 뒤 스택에 추가하고 뷰포트에 올린다.
>         2. `NotifyInputModeChange()`를 호출하여 입력 모드를 갱신한다.
> 

### 위젯 닫기(CloseUI) 상세 규칙

> 
> 
> 
> 
> - PERSISTENT 위젯이 닫히면 `PersistentUIMap`에서 제거하고 `Visibility`를 Collapsed로 전환한다.
> - POPUP 위젯이 닫히면 `PopupUIStack`에서 해당 위젯을 찾아 제거하고, 스택에 남은 최상위 위젯에 `OnFocusGained()`를 통보한다.
> - 이후 `NotifyInputModeChange()`를 호출하여 입력 모드를 재판단한다.
> 
> - 위젯 인스턴스가 스스로 닫힘을 요청하는 경우, `LRBaseWidget`의 `OnCloseRequested()`가 `OnCloseUIRequestedDel`을 Broadcast하고, 이 델리게이트는 `NativeConstruct` 시점에 UMS의 `CloseUI(ULRBaseWidget*)`에 자동 바인딩된다.
> - 즉 위젯은 자신이 닫히는 방법을 알 필요가 없으며 "닫기 요청"만 발생시키면 UMS가 나머지를 처리한다.
> 

### 입력 모드 전환 상세 규칙

> 
> 
> 
> 
> - 입력 모드는 `PopupUIStack` 상태를 기준으로 판단한다.
>     - 스택에 `bIsModal = true`인 위젯이 하나라도 있으면 `GameAndUI` 모드로 전환하고
>      마우스 커서를 표시한다.
>     - 스택에 모달 위젯이 없으면 `GameOnly` 모드로 전환하고 커서를 숨긴다.
> - 인게임에서 표시되는 HUD는 PERSISTENT이므로 이 판단에 영향을 주지 않는다.
>     - 로비 루트 위젯도 PERSISTENT이므로 입력 모드에 영향을 주지 않으며,
>     - 로비에서는 캐릭터 선택/스테이지 선택 POPUP이 열릴 때 `GameAndUI` 모드가 활성화된다.
> 
> | 모달 여부 | 입력 모드 | 마우스 커서 |
> | --- | --- | --- |
> | 모달 POPUP 존재 | GameAndUI | 표시 |
> | 모달 POPUP 없음 | GameOnly | 숨김 |
> 

### 레벨 전환 시 UI 처리 상세 규칙

> 
> 
> 
> 
> - 다음 레벨 열기(`GameInstance.OpenNextLevel()`)가 호출되면 레벨 전환 전에 반드시 모든 UI를 리셋한다.( `UMS.ResetAllUIStates()`)
>     - 이 함수는 열려있는 모든 위젯을 닫고, PopupUIStack과 PersistentUIMap, CachedWidgets를 전부 비운다.
>     - Transition 레벨에서 UIManager는 새로 초기화된 상태로 시작한다.
> - 로딩 화면은 UIManager가 아닌 **GameInstance가 직접 관리**한다.
>     - 이유는 Transition 레벨 진입 직후 UIManager가 완전히 초기화되기 전에도 로딩 화면이 즉시 표시되어야 하기 때문이다.
>     - `TransitionController.OpenFirstWidget()`이 UIManager를 통해 로딩 UI를 여는 시점까지는 GameInstance가 로딩 화면의 생명주기를 보장한다.
> 
> **예외처리:**
> 
> - `ResetAllUIStates()` 도중 강제 종료
>     - 레벨 전환이 완료되면 UIManager가 새로 초기화되므로 잔존 상태 없음
> - Transition 레벨에서 목적지 레벨 이름이 None인 경우
>     - 크래시. GameInstance에서 NextLevelName 설정 없이 전환 시도는 허용하지 않는다
> 

### 로비 내 화면 전환 상세 규칙

> 
> 
> 
> 
> - 로비 레벨에서 로비 루트 위젯은 PERSISTENT로 등록되어 항상 표시 상태를 유지한다.
> - 캐릭터 선택 화면과 스테이지 선택 화면은 각각 독립된 POPUP 위젯으로 스택에 쌓인다.
> - 전환 흐름
>     - 로비 레벨 시작 시 로비 루트 위젯이 PERSISTENT로 열린다.
>     - 플레이어가 캐릭터 선택 버튼을 누르면 캐릭터 선택 POPUP이 스택에 추가된다.
>         - 선택 완료 또는 뒤로 가기 시 캐릭터 선택 POPUP이 닫히고 로비 루트로 포커스가 돌아온다.
>     - 스테이지 선택도 동일한 방식으로 처리된다.
>         - 스테이지 선택이 완료되어 게임 시작이 확정되면 `GameInstance.OpenNextLevel(InGame)`을 호출하고, 
>         `ResetAllUIStates()` 이후 Transition 레벨로 이동한다.
> 

---

## 💡 위젯 목록 및 레이어 정의

프로젝트의 모든 위젯을 레이어, 모달 여부, 표시 레벨 기준으로 정리한다.

| 위젯 | 레이어 | bIsModal | 표시 레벨 | 열기 요청 시스템 |
| --- | --- | --- | --- | --- |
| 메인메뉴 위젯 | PERSISTENT | false | MainMenu | GameMode |
| 로비 루트 위젯 | PERSISTENT | false | Lobby | GameMode |
| 캐릭터 선택 위젯 | POPUP | true | Lobby | 로비 루트 위젯 |
| 스테이지 선택 위젯 | POPUP | true | Lobby | 로비 루트 위젯 |
| 인게임 HUD | PERSISTENT | false | InGame | GameMode |
| 레벨업 UI | POPUP | true | InGame | 레벨업 시스템 |
| 무기 교체 UI | POPUP | true | InGame | 레벨업 시스템 |
| 일시정지 메뉴 | POPUP | true | InGame | 플레이어 조작 시스템 |
| 결과 화면 | POPUP | true | InGame | 스테이지 시스템 |
| 리더보드 | POPUP | true | InGame | 결과 화면 위젯 |
| 로딩 화면 | (GameInstance 직접 관리) | — | Transition | TransitionController |

> **UMS 레이어 시스템 적용 제외 위젯**
>
> 아래 위젯은 PERSISTENT/POPUP 레이어 구조와 무관하게 동작하며, UMS를 거치지 않는다.
> "모든 위젯은 UMS를 통해서만" 규칙은 이 범주를 제외한다.
>
> | 위젯 | 관리 주체 | 제외 사유 |
> | --- | --- | --- |
> | `EnemyHPBarWidget` | `AEnemyBaseCharacter` (WidgetComponent) | 월드 스페이스 위젯. 뷰포트 레이어 시스템과 무관 |
> | `EnemyDamageWidget` | `ARSPlayerController` (직접 풀 소유) | 월드 이벤트 기반 일시적 UI 이펙트. 복수 인스턴스 동시 운용 필요로 UMS 단일 인스턴스 캐싱 구조와 양립 불가 |
>
> 정정 이력: 2026-03-26 — `PLAN_EnemyHPBar_DamageWidget_v1.0` 구현 계획서에 따라 추가

### 인게임 Popup 스택 예시

> 
> 
> 
> 
> - 인게임에서 팝업이 중첩되는 대표적인 시나리오
>     - 레벨업 이벤트 발생 시 스택은 `[레벨업 UI]`
>     - 슬롯이 가득 찬 경우 `[레벨업 UI, 무기 교체 UI]`로 전환.
>     - 교체 완료 시 두 위젯이 모두 닫히고 스택은 빈 상태로 복귀
>     - 스테이지 종료 시 `[결과 화면]`이 열리고, 
>     리더보드 버튼을 누르면 `[결과 화면, 리더보드]`로 쌓인다.
> 

---

## 💡 기반 위젯 클래스 설계 (BaseWidget)

> 
> 
> 
> ### 공개 인터페이스
> 
> | 함수 | 설명 |
> | --- | --- |
> | `InitializeUI()` | 위젯 최초 생성 시 1회 실행. 데이터 바인딩 초기화. |
> | `OpenUI()` | 화면에 표시. `RefreshUI()` 자동 호출 후 Visible로 전환. |
> | `CloseUI()` | 화면에서 숨김. Collapsed로 전환. |
> | `RefreshUI()` | 데이터 갱신. 열려있는 위젯의 내용을 최신 상태로 업데이트. |
> | `OnFocusGained()` | 팝업 스택 최상위가 되었을 때 호출. 자식 클래스에서 오버라이드. |
> | `OnFocusLost()` | 위에 다른 팝업이 열렸을 때 호출. 자식 클래스에서 오버라이드. |
> | `OnCloseRequested()` | 위젯이 스스로 닫힘을 요청. `OnCloseUIRequestedDel` Broadcast. |
> 
> ### UPROPERTY 설정값
> 
> | 프로퍼티 | 타입 | 설명 |
> | --- | --- | --- |
> | UILayer | EUILayer | PERSISTENT 또는 POPUP. 생성자에서 설정. |
> | ZOrder | INT | 동일 레이어 내 기본 ZOrder. 기본값 0. |
> | bIsModal | BOOL | true이면 입력 모드를 GameAndUI로 전환시킨다. |
> 
> ### NativeConstruct 자동 바인딩 규칙
> 
> - 모든 위젯은 `NativeConstruct` 시점에 `OnCloseUIRequestedDel`을 UMS의 `CloseUI(UVanguardBaseWidget*)`에 자동 바인딩한다.
> - 이미 바인딩되어 있으면 중복 등록하지 않는다.
> - 이 구조 덕분에 모든 자식 위젯은 닫힘 처리 방법을 알 필요 없이 `OnCloseRequested()`만 호출하면 된다.
> 

## 💡 데이터 스키마

> 
> 
> 
> 
> UMS는 별도의 DataTable을 사용하지 않는다. 
> 
> 위젯 클래스 매핑은 `UIManagerSettings` (DeveloperSettings 기반 에셋)에서 관리한다.
>
> ```cpp
> // UIManagerSettings 구조
> TMap<EUIID, TSoftClassPtr<URSBaseWidget>> UIClassMap;
> TMap<EUIID, EUILayer>                    UILayerMap;
> ```
>
> 이를 통해 `OpenUIByID(EUIID)` 방식으로 ID 기반 UI 오픈이 가능하다.
>
> **EUIID 전체 목록:**
>
> | EUIID | 레이어 | 설명 |
> | --- | --- | --- |
> | NONE | — | 자식 위젯 전용. 직접 오픈 불가. |
> | BACKGROUND | PERSISTENT | 배경 오버레이 |
> | LOADING | PERSISTENT | 로딩 화면 |
> | INTRO | PAGE | 인트로 화면 |
> | TITLE | PAGE | 타이틀 화면 |
> | OUTGAME | PERSISTENT | 아웃게임 공통 프레임 |
> | LOBBY | PAGE | 메인 로비 |
> | CHAR_SELECT | PAGE | 캐릭터 선택 |
> | STAGE_SELECT | PAGE | 스테이지 선택 |
> | SETTING | POPUP | 설정 메뉴 |
> | PAUSE | POPUP | 일시정지 메뉴 |
> | HUD | PERSISTENT | 인게임 HUD |
> | LEVEL_UP | POPUP | 레벨업 UI |
> | WEAPON_REPLACE | POPUP | 무기 교체 UI |
> | GAMEOVER | PAGE | 게임 오버 화면 |
> | GAMECLEAR | PAGE | 게임 클리어 화면 |
> | EXIT | SYSTEM | 종료 확인 다이얼로그 |
>
> **공개 API:**
>
> | API | 설명 |
> | --- | --- |
> | `OpenUIByID(EUIID)` | ID 기반 UI 오픈. UILayerMap 기준으로 레이어 자동 분기. |
> | `SwitchPageUI(EUIID)` | PAGE 레이어 전환. 현재 PAGE를 UIHistory에 push 후 새 PAGE 오픈. |
> | `BackPage()` | UIHistory에서 이전 PAGE를 pop하여 복귀. |
> | `ClearUIHistory()` | UIHistory 스택 초기화. 레벨 전환 시 호출. |
> | `CloseUIByID(EUIID)` | ID 기반 UI 닫기. PLAN_GameFlow_Data MODULE-4 마이그레이션용. |
> | `OpenUI<T>()` | 기존 클래스 기반 오픈 (하위 호환 유지). 신규 코드는 OpenUIByID 권장. |
>
> **레벨별 UI 요청자 원칙:**
>
> | 레벨 | UI 요청자 |
> | --- | --- |
> | INTRO 레벨 | RSIntroPlayerController |
> | TRANSITION 레벨 | RSTransitionPlayerController |
> | OUTGAME 레벨 | RSOutGamePlayerController |
> | INGAME 레벨 | RSPlayerController (→ EUIID 마이그레이션 후) |
>
> **마이그레이션 계획**: 기존 `OpenUI<T>()` 방식은 하위 호환을 위해 유지한다.
> 인게임 RSPlayerController → OpenUIByID(EUIID) 마이그레이션은 PLAN_GameFlow_Data MODULE-4에서 처리 예정.
>

### UMS 내부 캐시 구조

| 캐시 항목 | 타입 | 설명 |
| --- | --- | --- |
| PersistentUIMap | MAP<EUIID, Widget> | 현재 열려있는 PERSISTENT 위젯 |
| CurrentPageWidget | Widget | 현재 표시 중인 PAGE 위젯 |
| UIHistory | STACK<EUIID> | BackPage()용 PAGE 이력 스택 |
| PopupUIStack | ARRAY<Widget> | 현재 열려있는 POPUP 위젯. Last = 최상위 |
| SystemWidget | Widget | 현재 표시 중인 SYSTEM 위젯 (최대 1개) |
| CachedWidgets | MAP<EUIID, Widget> | 생성된 모든 위젯의 캐시. 닫혀도 보존 |

# 💡코드구현(code)