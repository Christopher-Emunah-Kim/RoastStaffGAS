---
name: commit
version: 1.1.0
depends-on: ["CODE+TEST+이슈수정 완료"]
allowed-tools: Read, Bash, Grep, Edit
---
# /commit RUNBOOK
> 역할: 전체 변경사항을 _Design/TODO.md MODULE 단위로 분할하여 순서대로 커밋 안내
> 타이밍: 모든 작업 완료 후 "커밋해줘" 발언 시만 실행

## ON_DEMAND_REFS
```yaml
policy: .claude/references/commit-policy.md  # 커밋 타입/형식/원칙 전체
```
> commit-policy.md는 타입/형식이 불명확할 때만 읽는다. 아래 요약으로 충분하면 생략.

### 커밋 타입 요약 (policy 읽기 전 참고)
```
<feat>     새 기능
<fix>      버그 수정
<data>     DataTable/CSV
<refactor> 리팩토링 (동작 변화 없음)
<docs>     문서
<chore>    설정/빌드
<scope>    시스템명 (예: feat(Pierce-Core): ...)
<body>     - 변경 이유/내용 (항목별)
<ref>      ref: PLAN_파일명_vX.X
```

## STATE_MACHINE
```
INIT ──→ [A] CHANGESET + TODO 읽기
          └─ [B] 파일 → MODULE 매핑
                └─ [C] 커밋 순서 + 메시지 일괄 제안 (ASK_USER_FORMAT)
                      ├─ 승인 → [D] Claude가 직접 커밋 실행
                      └─ 수정 → [C] 재조정
[D] → [E] 해시 자동 수집 → [F] CHANGESET+TODO 갱신 → DONE
```

## EXEC

### [A] 현황 파악
```
읽기:
- _Design/Changesets/CHANGESET.md (PENDING_COMMIT 항목)
- _Design/TODO.md (ACTIVE_WORK의 완료된 MODULE)

확인:
- 변경된 파일 전체 목록 (git status 또는 CHANGESET files 필드)
- MODULE별 담당 파일 (TODO MODULE 헤더의 신규/수정 필드)
```

### [B] 파일 → MODULE 매핑
```
MODULE-1 [이름]: [파일A.h, 파일A.cpp]
MODULE-2 [이름]: [파일B.h, DT_이름.csv]
미분류:          [파일D.h] ← 불명확 시 사용자 확인
```

### [C] 커밋 계획 제안 (ASK_USER_FORMAT)
```
📌 [COMMIT] | [기능명] — N개 커밋

커밋 1: feat(MODULE-1명): [요약]
  파일: [파일A.h, 파일A.cpp]
  메시지:
    feat(Pierce-Core): 관통 감지 구현
    - ECR_Overlap 충돌 설정
    ref: PLAN_Pierce_v1.0

커밋 2: data(MODULE-2명): [요약]
  ...

상황: 위 순서로 N개 커밋을 올립니다.
결정: 이 계획대로 진행할까요?
권장: A) — 모듈 순서가 TODO와 일치합니다.
A) 이대로 진행
B) 순서/메시지 조정
```

### [D] 직접 커밋 실행
승인 수령 후 커밋을 순서대로 직접 실행:
```bash
# 커밋 1
git add 파일A.h 파일A.cpp
git commit -m "<feat> Pierce-Core 관통 감지 구현

- ECR_Overlap 충돌 설정
- PierceHitCount 카운터 분리
ref: PLAN_Pierce_v1.0"

# 커밋 2 ... (이전 커밋 성공 확인 후 순차 실행)
```
- 각 커밋 실행 후 성공 여부 확인
- 실패 시 즉시 중단 + 오류 내용 보고

### [E] 해시 자동 수집
각 커밋 실행 결과에서 해시 자동 추출 (git log --oneline -1)

### [F] 일괄 갱신
```
_Design/Changesets/CHANGESET.md:
  commit: "abc1234"
  status: COMMITTED

_Design/TODO.md:
  [x] 태스크명 (abc1234)
  ### [MODULE-1] ✓ COMMITTED abc1234 YYYY-MM-DD
```
COMMITTED 항목 5개 초과 시 CHANGESET compact 제안.

## RULES
```
- "커밋해줘" 발언 전 커밋 언급 금지
- 미분류 파일은 사용자 확인 후 배정 (임의 배정 금지)
- 커밋 순서 = _Design/TODO.md MODULE 순서
- 해시 수령 전 CHANGESET/TODO 갱신 금지
- [ABSOLUTE] git commit 명령 실행 전 반드시 [C] 계획 제안 + 사용자 승인 수령
- [ABSOLUTE] 승인 없이 git commit 실행 금지 (어떤 상황에서도 예외 없음)
- [ABSOLUTE] 커밋 = 기능 완성 + 테스트 완료 상태. 중간 작업 상태 커밋 금지
```
