# PLAN_[시스템명]_v1.0
```yaml
date:    YYYY-MM-DD
sprint:  SPRINT-N
status:  ACTIVE          # ACTIVE | COMPLETED | ABANDONED
designs: [○○_시스템_기획_vX.X.md]
```

## GOAL
> 한 문장: 무엇을 왜 만드는가.

## SCOPE
```yaml
new_files:      []
modified_files: []
new_datatables: []
new_tags:       []
```

## FLOW
```
[진입점: 함수명/이벤트]
    │
    ▼
[Step 1: 함수A(인자)]
    ├─ 정상 ──→ [Step 2: 함수B]
    │                │
    │                ▼
    │           [Step 3: 출력/효과]
    └─ 예외1 ──→ [예외처리A] → RETURN/BLOCKED
    └─ 예외2 ──→ [예외처리B] → LOG + CONTINUE
```

## SCHEMA
```
DataTable: DT_[이름]
| 컬럼 | 타입 | FK | 기본값 | 설명 |
|------|------|----|--------|------|
```

## EDGE_CASES
```
| 상황 | 처리 | 기획서 근거 |
|------|------|------------|
```

## REVIEW_NOTES
```
기획서 일관성: ✓/✗ (불일치 항목)
누락 예외처리: (있으면 나열)
기획서 정정:   (있으면 나열)
Gemini 반영:   반영(사유) / 미반영(사유)
```

---
## REVIEW_STATUS
```
| 단계          | 상태 | 날짜       | 주요 지적         |
|---------------|------|------------|-------------------|
| Cross-Review  | -    | -          | -                 |
| Senior-Review | -    | -          | -                 |
| Learn-Report  | -    | -          | -                 |

verdict:   PENDING   # PENDING | CLEAR | BLOCKED
unresolved: []
```
