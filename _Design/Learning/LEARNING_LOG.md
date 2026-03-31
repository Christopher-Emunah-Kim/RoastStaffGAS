## 2026-03-31 — UE5BuildSetup
keywords: [UENUM-generated-h, DeveloperSettings-module, git-core-quotepath, UBT-Korean-filename-crash]
status:   🟡심화필요
note:     ① UENUM 있는 .h는 반드시 .generated.h 포함 ② UDeveloperSettings 쓰면 Build.cs에 "DeveloperSettings" 추가 ③ 한글 파일명 프로젝트엔 git config core.quotepath false 필수 (UBT가 octal 이스케이프 경로 파싱 시 .NET 크래시)

---

## 2026-03-27 — InputMode
keywords: [SetInputMode, SetShowMouseCursor, SetConsumeCaptureMouseDown, Slate-input-capture, diagnosis-order, git-diff-as-debug-tool]
status:   🔴미숙 (첫 진단 오류 — GAS 내부로 잘못 진입, 진입점 확인 생략)
score:    정합:N/A GAS:N/A 메모리:N/A OOP:N/A 컨벤션:N/A (SR 없음 — 자기주도 학습)
note:     삭제 편향(분기 검토 없는 한 줄 제거), 진단 순서 미준수, git diff 미활용 3가지 패턴 확인
report:   _Design/Learning/reports/LEARN_20260327_InputMode.md

---

## 2026-03-27 — UMGWidgetLifecycle
keywords: [NativeConstruct, NativeOnInitialized, AddDynamic, widget-caching, duplicate-binding]
status:   🔴미숙 (NativeOnInitialized 미사용 → 중복 바인딩 버그 발생, 수정 완료)
score:    정합:N/A GAS:N/A 메모리:N/A OOP:N/A 컨벤션:N/A (SR 없음 — 자기주도 학습)
report:   _Design/Learning/reports/LEARN_20260327_UMGWidgetLifecycle.md

---
