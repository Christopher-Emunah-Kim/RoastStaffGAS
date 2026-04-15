---
name: 반복 취약 패턴 — SR 기반 누적
description: 시니어 리뷰에서 2회 이상 지적된 항목 및 RESOLVED 이력. 다음 세션 학습 리포트 작성 시 교차 참조.
type: user
---

## RESOLVED (완전 체화 확인)

| 패턴 | 해소 세션 | 비고 |
|------|-----------|------|
| UPROPERTY GC 추적 누락 | 2026-04-15 PassiveSlotUI | 2회 지적 후 완전 클리어. LoadedPassiveIcon 포함 모든 멤버 정상 적용. |
| AddDynamic NativeOnInitialized 배치 | 2026-04-15 PassiveSlotUI | 자기주도 발견(2026-03-27) 후 RESOLVED 확정. |

## RECURRING (현재 진행형 취약점)

| 패턴 | 횟수 | 최초 지적 | 최근 지적 | 비고 |
|------|------|-----------|-----------|------|
| 하드코딩 수치 | 4회 | (SR-FULL 이전) | 2026-04-08 SR-FULL | 최우선 교정 대상. DataTable 스키마 먼저 설계 습관 필요. |
| ensureMsgf 후 nullptr guard 없는 역참조 | 1회 | 2026-04-15 PassiveSlotUI | 2026-04-15 | Shipping 빌드에서 크래시 위험. RECURRING 진입 신호. |
| include 경로 대소문자 혼용 | 1회 | 2026-04-15 PassiveSlotUI | 2026-04-15 | Linux/Mac 빌드 에러 원인. RECURRING 진입 신호. |

## 애매해 항목 (이번 세션 미해소)

| 패턴 | 설명 날짜 | 다음 확인 목표 |
|------|-----------|----------------|
| LoadSynchronous + UPROPERTY GC 강참조 | 2026-04-15 | 즉답 가능 수준 — 로컬 변수 저장 금지 이유 자력 설명 |
