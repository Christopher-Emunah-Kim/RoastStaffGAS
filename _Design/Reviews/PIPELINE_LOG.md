# PIPELINE_LOG — 파이프라인 자가 진단 기록
> SESSION_END [PR] 단계에서 자동 기록.
> 패턴이 3회 이상 반복되면 구조적 문제로 판단 → 즉시 개선 제안.

---

## 로그

## 2026-04-07
관찰: EXPLAIN_IMPL 능동 진단 포함 여부 1회 왕복. 초기 제안 명세 불충분.
개선: 없음 (건너뛰기)

## 2026-04-10
관찰: PIPELINE_FLOW 섹션 인라인→외부파일 이동 시 EXPLAIN_IMPL 줄 소실 (시니가 직접 발견). CONSTRAINTS 위치 논쟁 2회 왕복.
개선: CLAUDE.md PIPELINE 섹션 EXPLAIN_IMPL 줄에 [고정] 마커 추가 — 편집 시 삭제 방지
